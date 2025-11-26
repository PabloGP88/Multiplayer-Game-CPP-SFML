//
// Created by Pablo Gonzalez Poblette on 16/11/25.
//

#include "game_server.h"
#include "../game/utils.h"
#include "../game/protocole_message.h"
#include <thread>

game_server::game_server(unsigned short port)
    : collisionManager(1280.f, 960.f)
{
    if (socket.bind(port) != sf::Socket::Status::Done) {
        Utils::printMsg("Failed to bind server to port " + std::to_string(port), error);
        throw std::runtime_error("Server bind failed");
    }

    socket.setBlocking(false);

    CreateObstacles();

    Utils::printMsg("=== SERVER STARTED ===", success);
    Utils::printMsg("Port: " + std::to_string(port), info);
    Utils::printMsg("Tick Rate: " + std::to_string(TICK_RATE) + " Hz", info);
    Utils::printMsg("Obstacles created: " + std::to_string(obstacles.size()), success);
}

void game_server::Update() {
    sf::Clock clock;
    float tickTime = 1.0f / TICK_RATE;
    float accumulator = 0.0f;

    Utils::printMsg("Waiting for players to join...", success);

    while (true) {
        float dt = clock.restart().asSeconds();
        accumulator += dt;

        // Fixed timestep updates
        while (accumulator >= tickTime) {
            ProcessMessages();
            CheckClientTimeouts();
            CheckPendingRespawns();
            accumulator -= tickTime;
        }

        // Send game state to all clients
        SendGameSnapShot();

        // Sleep to prevent CPU hogging
        sf::sleep(sf::milliseconds(1));
    }
}

void game_server::ProcessMessages() {
    sf::Packet packet;
    std::optional<sf::IpAddress> senderIP;
    unsigned short senderPort;

    while (socket.receive(packet, senderIP, senderPort) == sf::Socket::Status::Done) {
        uint8_t typeValue;
        if (!(packet >> typeValue)) continue;

        MessageTypeProtocole type = static_cast<MessageTypeProtocole>(typeValue);

        switch (type) {
            case MessageTypeProtocole::JOIN_REQUEST: {
                JoinRequestMessage msg;
                if (packet >> msg) {
                    HandleJoinRequest(senderIP.value(), senderPort, msg);
                }
                break;
            }

        case MessageTypeProtocole::TANK_UPDATE: {
                TankMessage msg;
                if (packet >> msg) {
                    // Update heartbeat
                    auto it = clients.find(msg.playerId);
                    if (it != clients.end()) {
                        it->second.lastHeartbeat.restart();
                    }
                    HandleTankUpdate(msg);
                }
                break;
            }

            case MessageTypeProtocole::DISCONNECT: {
                // Handle explicit disconnect
                int playerId;
                if (packet >> playerId) {
                    HandleDisconnect(playerId);
                }
                break;
            }

            default:
                Utils::printMsg("Unknown message type: " + std::to_string(typeValue), warning);
                break;
        }
    }
}

void game_server::HandleJoinRequest(sf::IpAddress sender, unsigned short port, JoinRequestMessage msg) {
    int playerId = nextPlayerId++;
    std::string color = AssignColor();

    // Create client info
    clients.try_emplace(playerId, sender, port, playerId);
    clients.at(playerId).lastHeartbeat.restart();

    tanks[playerId] = std::make_unique<Tank>(color);
    tanks[playerId]->position = {640, 480};

    // Send acceptance to joining client
    JoinAcceptedMessage acceptMsg;
    acceptMsg.assignedPlayerId = playerId;
    acceptMsg.tankColor = color;

    sf::Packet acceptPacket;
    acceptPacket << static_cast<uint8_t>(MessageTypeProtocole::JOIN_ACCEPTED) << acceptMsg;
    socket.send(acceptPacket, sender, port);

    SendObstaclesPosition(playerId);

    // Notify all other clients about new player
    PlayerJoinedMessage joinMsg;
    joinMsg.playerId = playerId;
    joinMsg.color = color;

    sf::Packet joinPacket;
    joinPacket << static_cast<uint8_t>(MessageTypeProtocole::PLAYER_JOINED) << joinMsg;
    BroadcastMessage(joinPacket);

    Utils::printMsg("Player " + std::to_string(playerId) + " (" + color + ") joined from " +
                   sender.toString() + ":" + std::to_string(port), success);
}

void game_server::HandleTankUpdate(TankMessage msg) {

    auto tankIt = tanks.find(msg.playerId);
    if (tankIt == tanks.end()) return;

    Tank* tank = tankIt->second.get();

    // update from client data
    tank->position = {msg.x, msg.y};
    tank->bodyRotation = sf::degrees(msg.rotationBody);
    tank->barrelRotation = sf::degrees(msg.rotationBarrel);

    // Check if player just died and isn't already pending respawn
    auto clientIt = clients.find(msg.playerId);
    if (clientIt != clients.end()) {
        if (!msg.isAlive && !clientIt->second.isPendingRespawn) {
            Utils::printMsg("Player " + std::to_string(msg.playerId) + " died", error);

            // Mark as pending respawn to prevent duplicate entries
            clientIt->second.isPendingRespawn = true;

            // Broadcast death
            PlayerDiedMessage diedMsg;
            diedMsg.victimId = msg.playerId;

            sf::Packet deathPacket;
            deathPacket << static_cast<uint8_t>(MessageTypeProtocole::PLAYER_DIED) << diedMsg;
            BroadcastMessage(deathPacket);

            // Add to respawn queue, 2 second timer
            pendingRespawns.emplace_back(msg.playerId, -1);
        }

        // Perform shooting just once per press
        bool prevState = clientIt->second.prevShootState;
        if (msg.shootPressed && !prevState && tank->getAmmo() > 0) {
            SpawnBullet(msg.playerId);
        }
        clientIt->second.prevShootState = msg.shootPressed;
    }
}

void game_server::HandleDisconnect(int playerId) {
    auto clientIt = clients.find(playerId);
    if (clientIt == clients.end()) return;

    // Free the color
    auto tankIt = tanks.find(playerId);
    if (tankIt != tanks.end()) {
        FreeColor(tankIt->second->GetColor());
        tanks.erase(tankIt);
    }

    // Remove client
    clients.erase(clientIt);

    // Notify all clients
    PlayerLeftMessage leftMsg;
    leftMsg.playerId = playerId;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::PLAYER_LEFT) << leftMsg;
    BroadcastMessage(packet);

    Utils::printMsg("Player " + std::to_string(playerId) + " disconnected", warning);
}

void game_server::SpawnBullet(int ownerId) {
    auto tankIt = tanks.find(ownerId);
    if (tankIt == tanks.end()) return;

    Tank* tank = tankIt->second.get();
    if (tank->getAmmo() <= 0) return;

    // Calculate bullet spawn position
    float tipRotation = (tank->barrelRotation + sf::degrees(90)).asRadians();
    sf::Vector2f barrelTip = tank->position + sf::Vector2f{
        std::cos(tipRotation) * 30.f,  // barrel length
        std::sin(tipRotation) * 30.f
    };

    // Create bullet
    ServerBullet serverBullet;
    serverBullet.bulletId = nextBulletId++;
    serverBullet.ownerId = ownerId;
    serverBullet.bulletObj = std::make_unique<bullet>(barrelTip, tank->barrelRotation);
    int bulletId = serverBullet.bulletId;
    bullets.push_back(std::move(serverBullet));


    // Broadcast bullet spawn
    BulletSpawnedMessage msg;
    msg.bulletId = bulletId;
    msg.x = barrelTip.x;
    msg.y = barrelTip.y;
    msg.rotation = tank->barrelRotation.asDegrees();
    msg.ownerId = ownerId;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::BULLET_SPAWNED) << msg;
    BroadcastMessage(packet);

    Utils::printMsg("Player " + std::to_string(ownerId) + " fired bullet " +
                   std::to_string(serverBullet.bulletId), debug);
}

void game_server::SendGameSnapShot() {
    GameStateMessage state = BuildGameState();

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::GAME_STATE) << state;

    BroadcastMessage(packet);
}

GameStateMessage game_server::BuildGameState() {
    GameStateMessage state;

    // Add all players

    for (const auto& [id, tank] : tanks) {
        GameStateMessage::PlayerState p;
        p.playerId = id;
        p.x = tank->position.x;
        p.y = tank->position.y;
        p.rotationBody = tank->bodyRotation.asDegrees();
        p.rotationBarrel = tank->barrelRotation.asDegrees();
        p.health = tank->getHealth();
        p.ammo = tank->getAmmo();
        p.isAlive = tank->IsAlive();
        p.color = tank->GetColor();
        state.players.push_back(p);
    }

    return state;
}

void game_server::CheckClientTimeouts() {
    std::vector<int> timedOutClients;

    for (const auto& [id, client] : clients) {
        if (client.lastHeartbeat.getElapsedTime().asSeconds() > CLIENT_TIMEOUT) {
            timedOutClients.push_back(id);
        }
    }

    for (int id : timedOutClients) {
        Utils::printMsg("Player " + std::to_string(id) + " timed out", warning);
        HandleDisconnect(id);
    }
}

void game_server::BroadcastMessage(sf::Packet& packet) {
    for (const auto& [id, client] : clients) {
        socket.send(packet, client.ipAddress, client.port);
    }
}

void game_server::SendToClient(int playerId, sf::Packet& packet) {
    auto it = clients.find(playerId);
    if (it != clients.end()) {
        socket.send(packet, it->second.ipAddress, it->second.port);
    }
}

std::string game_server::AssignColor() {
    for (size_t i = 0; i < availableColors.size(); i++) {
        if (!colorUsed[i]) {
            colorUsed[i] = true;
            return availableColors[i];
        }
    }

    printf("NO COLOR AVILABLE");
    return "blue";  // Fallback
}

void game_server::FreeColor(const std::string& color) {
    for (size_t i = 0; i < availableColors.size(); i++) {
        if (availableColors[i] == color) {
            colorUsed[i] = false;
            break;
        }
    }
}

void game_server::CreateObstacles()
{
    int numRocks = 10;
    int minSize = 3;
    int maxSize = 6;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(0.f, 800.f);
    std::uniform_real_distribution<float> posY(0.f, 600);
    std::uniform_int_distribution<int> sizeDist(minSize, maxSize);

    sf::Vector2f spawnPoint(640.f, 480.f);
    float minSpawnClearance = 100.f;

    for (int i = 0; i < numRocks; i++)
    {
        sf::Vector2f pos;
        bool validPosition = false;

        // To ensure center is clear
        while (!validPosition) {
            pos = { posX(gen), posY(gen) };

            float distToSpawn = std::sqrt(
                std::pow(pos.x - spawnPoint.x, 2) +
                std::pow(pos.y - spawnPoint.y, 2)
            );

            if (distToSpawn > minSpawnClearance) {
                validPosition = true;
            }
        }

        int size = sizeDist(gen);
        sf::Vector2f scale(size, size);

        auto rock = std::make_unique<obstacle>("Assets/Rock.png", pos,
                      sf::Vector2f(0,0), sf::Vector2f(0,0), scale);

        collisionManager.AddStaticCollider(rock->GetBounds());

        std::uniform_int_distribution<int> brightness(100, 255);
        sf::Color tint(brightness(gen), brightness(gen), brightness(gen));
        rock->sprite.setColor(tint);

        obstacles.push_back(std::move(rock));
    }
}

void game_server::CreatePowerUps()
{
    int numHealthKits = 2;
    int numAmmoBoxes = 4;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(0.f, 800.f);
    std::uniform_real_distribution<float> posY(0.f, 600.f);

    float rockSpace = 32.0f;

    // Create health kits
    for (int i = 0; i < numHealthKits; i++)
    {
        sf::Vector2f pos;
        bool validPosition = false;

        while (!validPosition)
        {
            pos = { posX(gen), posY(gen) };
            validPosition = true;

            for (auto& obs : obstacles)
            {
                float distToSpawn = std::sqrt(
                    std::pow(pos.x - obs->GetPosition().x, 2) +
                    std::pow(pos.y - obs->GetPosition().y, 2)
                );

                if (distToSpawn < rockSpace) {
                    validPosition = false;
                    break;  // No need to check other obstacles
                }
            }
        }


        auto healthKit = std::make_unique<class healthKit>(pos);
        healthKits.push_back(std::move(healthKit));
    }

    // Create ammo boxes with same logic
    for (int i = 0; i < numAmmoBoxes; i++)
    {
        sf::Vector2f pos;
        bool validPosition = false;

        while (!validPosition)
        {
            pos = { posX(gen), posY(gen) };
            validPosition = true;

            for (auto& obs : obstacles)
            {
                float distToSpawn = std::sqrt(
                    std::pow(pos.x - obs->GetPosition().x, 2) +
                    std::pow(pos.y - obs->GetPosition().y, 2)
                );

                if (distToSpawn < rockSpace) {
                    validPosition = false;
                    break;
                }
            }
        }

        auto ammoBox = std::make_unique<class ammoBox>(pos);
        ammoBoxes.push_back(std::move(ammoBox));
    }
}

void game_server::SendPickUpsPosition(int playerId)
{
    PickUpMessage msg;

    for (size_t i = 0; i < healthKits.size(); i++)
    {
        PickUpMessage::PickUpData data;
        data.pickUpId = i;
        data.pickUpType = 1; // HealthKit
        data.x = healthKits[i]->GetPosition().x;
        data.y = healthKits[i]->GetPosition().y;
        msg.pickUps.push_back(data);
    }

    for (size_t i = 0; i < ammoBoxes.size(); i++)
    {
        PickUpMessage::PickUpData data;
        data.pickUpId = healthKits.size() + i;
        data.pickUpType = 0; // AmmoBox
        data.x = ammoBoxes[i]->GetPosition().x;
        data.y = ammoBoxes[i]->GetPosition().y;
        msg.pickUps.push_back(data);
    }

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::PickUP_DATA) << msg;
    SendToClient(playerId, packet);
}

void game_server::SendObstaclesPosition(int playerId)
{
    ObstacleSpawnedMessage obs;

    for (const auto& obstacle: obstacles)
    {
        ObstacleSpawnedMessage::ObstacleData d;

        d.x = obstacle->GetPosition().x;
        d.y = obstacle->GetPosition().y;
        d.width = obstacle->GetBounds().size.x;
        d.height = obstacle->GetBounds().size.y;
        d.scaleX = obstacle->GetScale().x;
        d.scaleY = obstacle->GetScale().y;
        d.texture = obstacle->GetTexturePath();

        obs.obstacles.push_back(d);
    }

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::OBSTACLE_DATA) << obs;

    SendToClient(playerId, packet);

    Utils::printMsg("Sent obstacle stuff to client", debug);
}

void game_server::CheckPendingRespawns()
{
    for (auto i = pendingRespawns.begin(); i != pendingRespawns.end();)
    {
        if (i->deathTimer.getElapsedTime().asSeconds() >= RESPAWN_TIME)
        {
            RespawnPlayer(i->victimId);
            i = pendingRespawns.erase(i);
        } else
        {
            ++i;
        }
    }
}

void game_server::RespawnPlayer(int playerId) {
    auto tankIt = tanks.find(playerId);
    if (tankIt == tanks.end()) {
        Utils::printMsg("Tank with id:  " + std::to_string(playerId) + " - not found in the vector", error);
        return;
    }

    Tank* tank = tankIt->second.get();

    // Respawn at center
    sf::Vector2f respawnPosition = {640.f, 480.f};
    tank->position = respawnPosition;

    auto clientIt = clients.find(playerId);
    if (clientIt != clients.end()) {
        clientIt->second.prevShootState = false;
        clientIt->second.isPendingRespawn = false;
    }

    Utils::printMsg("Bro with id: " + std::to_string(playerId) + " back in action", success);

    // Send respawn notification to all clients
    PlayerRespawnedMessage respawnMsg;
    respawnMsg.playerId = playerId;
    respawnMsg.x = respawnPosition.x;
    respawnMsg.y = respawnPosition.y;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::PLAYER_RESPAWNED) << respawnMsg;
    BroadcastMessage(packet);
}
