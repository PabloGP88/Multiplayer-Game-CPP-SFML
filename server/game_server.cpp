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
    Utils::printMsg("=== DEDICATED SERVER STARTED ===", success);
    Utils::printMsg("Port: " + std::to_string(port), info);
    Utils::printMsg("Tick Rate: " + std::to_string(TICK_RATE) + " Hz", info);
}

void game_server::Update() {
    sf::Clock clock;
    float tickTime = 1.0f / TICK_RATE;
    float accumulator = 0.0f;

    Utils::printMsg("Server running... waiting for players", success);

    while (true) {
        float dt = clock.restart().asSeconds();
        accumulator += dt;

        // Fixed timestep updates
        while (accumulator >= tickTime) {
            ProcessMessages();
            UpdateSnapShot(tickTime);
            CheckClientTimeouts();
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

        case MessageTypeProtocole::CLIENT_INPUT: {
                ClientInputMessage msg;
                if (packet >> msg) {
                    // Update heartbeat
                    auto it = clients.find(msg.playerId);
                    if (it != clients.end()) {
                        it->second.lastHeartbeat.restart();
                    }
                    HandleClientInput(msg);
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

void game_server::HandleClientInput(ClientInputMessage msg) {
    auto tankIt = tanks.find(msg.playerId);
    if (tankIt == tanks.end()) return;

    Tank* tank = tankIt->second.get();

    // Apply movement input
    tank->isMoving.forward = msg.moveForward;
    tank->isMoving.backward = msg.moveBackward;
    tank->isMoving.left = msg.turnLeft;
    tank->isMoving.right = msg.turnRight;

    // Apply aiming inputs
    tank->isAiming.left = msg.aimLeft;
    tank->isAiming.right = msg.aimRight;

    // Handle shooting
    if (msg.shootPressed) {
        SpawnBullet(msg.playerId);
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

void game_server::UpdateSnapShot(float dt) {
    collisionManager.ClearDynamicColliders();

    // Update all tanks
    for (auto& [id, tank] : tanks) {
        tank->Update(dt, collisionManager);
    }

    // Update all bullets
    UpdateBullets(dt);

    // Check bullet collisions
    CheckBulletCollisions();
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
    serverBullet.bulletObj = std::make_unique<bullet>(barrelTip, tank->barrelRotation, 400.f);
    bullets.push_back(std::move(serverBullet));

    // Decrease ammo
    tank->AddAmmo(-1);

    // Broadcast bullet spawn
    BulletSpawnedMessage msg;
    msg.bulletId = serverBullet.bulletId;
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

void game_server::UpdateBullets(float dt) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->bulletObj->Update(dt, collisionManager);

        if (!it->bulletObj->IsActive()) {
            Utils::printMsg("Bullet " + std::to_string(it->bulletId) + " destroyed (wall)", debug);
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void game_server::CheckBulletCollisions() {
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        bool bulletDestroyed = false;

        // Check against all tanks except the owner
        for (auto& [tankId, tank] : tanks) {
            if (tankId == bulletIt->ownerId) continue;
            if (!tank->IsAlive()) continue;

            if (bulletIt->bulletObj->CheckTankCollision(tank.get())) {
                // HIT!
                int damage = bulletIt->bulletObj->GetDamage();
                tank->TakeDamage(damage);

                // Send hit message
                PlayerHitMessage hitMsg;
                hitMsg.victimId = tankId;
                hitMsg.shooterId = bulletIt->ownerId;
                hitMsg.damage = damage;
                hitMsg.newHealth = tank->getHealth();

                sf::Packet packet;
                packet << static_cast<uint8_t>(MessageTypeProtocole::PLAYER_HIT) << hitMsg;
                BroadcastMessage(packet);

                Utils::printMsg("Player " + std::to_string(bulletIt->ownerId) +
                               " hit player " + std::to_string(tankId) +
                               " for " + std::to_string(damage) + " damage", warning);

                if (!tank->IsAlive()) {
                    Utils::printMsg("Player " + std::to_string(tankId) + " died!", error);
                }

                bulletDestroyed = true;
                break;
            }
        }

        if (bulletDestroyed) {
            bulletIt = bullets.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }
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

    // Add all bullets
    for (const auto& serverBullet : bullets) {
        GameStateMessage::BulletState b;
        b.bulletId = serverBullet.bulletId;
        b.x = serverBullet.bulletObj->GetPosition().x;
        b.y = serverBullet.bulletObj->GetPosition().y;
        b.rotation = 0;  // Need to expose this from bullet class
        b.ownerId = serverBullet.ownerId;
        state.bullets.push_back(b);
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