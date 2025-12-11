//
// Created by Pablo Gonzalez Poblette on 17/11/25.
//

#include "client_main.h"
#include "../game/utils.h"

client_main::client_main(sf::IpAddress serverIp, unsigned short serverPort)
    : serverIp(serverIp), serverPort(serverPort), isConnected(false), playerId(-1)
{
    if (socketUDP.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        Utils::printMsg("Failed to bind UDP socket", error);
    }
    socketUDP.setBlocking(false);
}

bool client_main::Connect()
{
    // FIRST, try TCP connection in BLOCKING mode
    socketTCP.setBlocking(true);

    sf::Socket::Status status = socketTCP.connect(serverIp, serverPort, sf::seconds(10));

    if (status != sf::Socket::Status::Done)
    {
        Utils::printMsg("Failed to connect to server via TCP", error);
        return false;
    }

    Utils::printMsg("TCP connected to server", success);

    // NOW set to non-blocking for message processing
    socketTCP.setBlocking(false);

    Utils::printMsg("UDP Port: " + std::to_string(socketUDP.getLocalPort()), debug);

    JoinRequestMessage joinMsg;
    joinMsg.udpPort = socketUDP.getLocalPort();

    std::cout << "Enter player name: ";
    std::getline(std::cin, joinMsg.playerName);

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::JOIN_REQUEST) << joinMsg;

    if (socketTCP.send(packet) != sf::Socket::Status::Done)
    {
        Utils::printMsg("Failed to send join request", error);
        return false;
    }

    Utils::printMsg("Join request sent, waiting for response...", info);

    // Wait for response with timeout
    sf::Clock timeout;
    while (timeout.getElapsedTime().asSeconds() < 5.0f)
    {
        sf::Packet responsePacket;
        sf::Socket::Status receiveStatus = socketTCP.receive(responsePacket);

        if (receiveStatus == sf::Socket::Status::Done)
        {
            uint8_t typeValue;
            if (responsePacket >> typeValue)
            {
                MessageTypeProtocole type = static_cast<MessageTypeProtocole>(typeValue);

                if (type == MessageTypeProtocole::JOIN_ACCEPTED)
                {
                    JoinAcceptedMessage acceptMsg;
                    if (responsePacket >> acceptMsg)
                    {
                        HandleJoinAccepted(acceptMsg);
                        return true;
                    }
                }
                else if (type == MessageTypeProtocole::JOIN_REJECTED)
                {
                    JoinRejectedMessage rejectMsg;
                    if (responsePacket >> rejectMsg)
                    {
                        Utils::printMsg(rejectMsg.message, error);
                        return false;
                    }
                }
            }
        }
        else if (receiveStatus == sf::Socket::Status::Disconnected)
        {
            Utils::printMsg("Server disconnected during join", error);
            return false;
        }

        sf::sleep(sf::milliseconds(100));
    }

    Utils::printMsg("Connection timeout - server did not respond", error);
    return false;
}

void client_main::Disconnect()
{
    if (!isConnected)
        return;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::DISCONNECT) << playerId;
    socketUDP.send(packet, serverIp, serverPort);

    isConnected = false;
    Utils::printMsg("Disconnected from server", warning);
}

void client_main::Update()
{
    ReceiveMessages();
    ReceiveMessagesTCP();
    SendPosition();
}

void client_main::SendPosition()
{
    if (!isConnected || !game || playerId == -1)
        return;

    if (sendClock.getElapsedTime().asSeconds() < SEND_RATE)
        return;

    sendClock.restart();

    TankMessage msg = TankPositionMessage();

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::TANK_UPDATE) << msg;

    if (socketUDP.send(packet, serverIp, serverPort) != sf::Socket::Status::Done)
    {
        Utils::printMsg("UDP SEND FAILED", error);
    }
}

void client_main::ReceiveMessagesTCP()
{
    if (!isConnected)
        return;

    sf::Packet packet;

    while (socketTCP.receive(packet) == sf::Socket::Status::Done)
    {
        uint8_t typeValue;
        if (!(packet >> typeValue))
            continue;

        MessageTypeProtocole type = static_cast<MessageTypeProtocole>(typeValue);

        switch (type)
        {
            case MessageTypeProtocole::PLAYER_JOINED:
            {
                PlayerJoinedMessage msg;
                if (packet >> msg)
                {
                    HandlePlayerJoined(msg);
                }
                break;
            }

            case MessageTypeProtocole::PLAYER_LEFT:
            {
                PlayerLeftMessage msg;
                if (packet >> msg)
                {
                    HandlePlayerLeft(msg);
                }
                break;
            }
            case MessageTypeProtocole::OBSTACLE_SEED:
            {
                ObstacleSeedMessage msg;
                if (packet >> msg)
                {
                    HandleObstacles(msg);
                }
                break;
            }
            case MessageTypeProtocole::PickUP_DATA:
                {
                    PickUpMessage msg;
                    if (packet >> msg)
                    {
                        HandlePickUpData(msg);
                    }
                    break;
                }

            default:
                Utils::printMsg("Unknown message type: " + std::to_string(typeValue), warning);
                break;
        }
    }
}

void client_main::ReceiveMessages()
{
    if (!isConnected)
        return;

    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while (socketUDP.receive(packet, sender, port) == sf::Socket::Status::Done)
    {
        uint8_t typeValue;
        if (!(packet >> typeValue))
            continue;

        MessageTypeProtocole type = static_cast<MessageTypeProtocole>(typeValue);

        switch (type)
        {
            case MessageTypeProtocole::GAME_STATE:
            {
                GameStateMessage msg;
                if (packet >> msg)
                {
                    HandleGameSnapShot(msg);
                }
                break;
            }

            case MessageTypeProtocole::PLAYER_JOINED:
            {
                PlayerJoinedMessage msg;
                if (packet >> msg)
                {
                    HandlePlayerJoined(msg);
                }
                break;
            }

            case MessageTypeProtocole::PLAYER_LEFT:
            {
                PlayerLeftMessage msg;
                if (packet >> msg)
                {
                    HandlePlayerLeft(msg);
                }
                break;
            }

            case MessageTypeProtocole::BULLET_SPAWNED:
            {
                BulletSpawnedMessage msg;
                if (packet >> msg)
                {
                    HandleBulletSpawned(msg);
                }
                break;
            }

            case MessageTypeProtocole::OBSTACLE_SEED:
            {
                ObstacleSeedMessage msg;
                if (packet >> msg)
                {
                    HandleObstacles(msg);
                }
                break;
            }

            case MessageTypeProtocole::PLAYER_DIED:
                {
                    PlayerDiedMessage msg;
                    if (packet >> msg)
                    {
                        HandlePlayerDied(msg);
                    }
                    break;
                }

            case MessageTypeProtocole::PLAYER_RESPAWNED:
                {
                    PlayerRespawnedMessage msg;
                    if (packet >> msg)
                    {
                        HandlePlayerRespawned(msg);
                    }
                    break;
                }

            case MessageTypeProtocole::PickUP_DATA:
                {
                    PickUpMessage msg;
                    if (packet >> msg)
                    {
                        HandlePickUpData(msg);
                    }
                    break;
                }
            case MessageTypeProtocole::PickUp_UPDATE:
                {
                    PickUpUpdatedMessage msg;
                    if (packet >> msg)
                    {
                        HandlePickUpUpdated(msg);
                    }
                    break;
                }

            default:
                Utils::printMsg("Unknown message type: " + std::to_string(typeValue), warning);
                break;
        }
    }
}

void client_main::HandleJoinAccepted(JoinAcceptedMessage msg)
{
    playerId = msg.assignedPlayerId;
    playerColour = msg.tankColor;
    isConnected = true;

    game = std::make_unique<Game>(playerId);
    game->AddTank(playerId, playerColour);

    game->OnPickupCollected = [this](uint8_t pickupId, uint8_t pickupType) {
        SendPickupHit(pickupId, pickupType);
    };
    Utils::printMsg("Connected Player ID: " + std::to_string(playerId) +
                   " Color: " + playerColour, success);
}

void client_main::HandleGameSnapShot(GameStateMessage msg)
{
    for (const auto& playerState : msg.players)
    {
        // NOT MESS WITH LOCAL
        if (playerState.playerId == playerId)
            continue;

        // Add tank if missing
        if (game->tanks.find(playerState.playerId) == game->tanks.end())
        {
            game->AddTank(playerState.playerId, playerState.color);
        }

        // Store data for interpo
        game->AddNetworkTankState(playerState.playerId, playerState);
    }
}

void client_main::HandlePlayerJoined(PlayerJoinedMessage msg)
{
    if (!game || msg.playerId == playerId)
        return;

    game->AddTank(msg.playerId, msg.color);
    Utils::printMsg("Player " + std::to_string(msg.playerId) + " joined (" + msg.color + ")", info);
}

void client_main::HandlePlayerLeft(PlayerLeftMessage msg)
{
    if (!game)
        return;

    auto it = game->tanks.find(msg.playerId);
    if (it != game->tanks.end())
    {
        game->tanks.erase(it);
        Utils::printMsg("Player " + std::to_string(msg.playerId) + " left", warning);
    }
}

void client_main::HandleBulletSpawned(BulletSpawnedMessage msg)
{
    if (!game)
        return;

    // Create bullet at the position specified by server
    sf::Vector2f bulletPos(msg.x, msg.y);
    sf::Angle bulletRotation = sf::degrees(msg.rotation);

    // Find the owner tank
    auto tankIt = game->tanks.find(msg.ownerId);
    if (tankIt != game->tanks.end())
    {
        // Add bullet to the tank's bullet list
        tankIt->second->bullets.push_back(
            std::make_unique<bullet>(bulletPos, bulletRotation, 400.f)
        );


        tankIt->second->DecreaseAmmo(1); // Reduce owner's ammo
    }
}

TankMessage client_main::TankPositionMessage()
{
    if (!game || playerId == -1)
        return {};

    TankMessage msg;

    msg.playerId = playerId;
    msg.x = game->tanks[playerId]->position.x;
    msg.y = game->tanks[playerId]->position.y;
    msg.rotationBody = game->tanks[playerId]->bodyRotation.asDegrees();
    msg.rotationBarrel = game->tanks[playerId]->barrelRotation.asDegrees();

    msg.shootPressed = game->tanks[playerId]->wantsToShoot;

    msg.isAlive = game->tanks[playerId]->IsAlive();

    return msg;
}

void client_main::HandleObstacles(ObstacleSeedMessage msg)
{
    if (!game)
        return;

    Utils::printMsg("Obs data received mate", debug);

    int numRocks = 10;
    int minSize = 3;
    int maxSize = 6;

    std::mt19937 gen(msg.seed);
    std::uniform_real_distribution<float> posX(0.f, 800.f);
    std::uniform_real_distribution<float> posY(0.f, 600);
    std::uniform_int_distribution<int> sizeDist(minSize, maxSize);

    sf::Vector2f spawnPoint(640.f, 480.f);
    float minSpawnClearance = 100.f;

    for (int i = 0; i < numRocks; i++)
    {
        sf::Vector2f pos;
        bool validPosition = false;

        while (!validPosition)
        {
            pos = {posX(gen),posY(gen)};

            float dist = std::sqrt(
                std::pow(pos.x - spawnPoint.x, 2) + std::pow(pos.y - spawnPoint.y, 2)
                );

            if (dist > minSpawnClearance)
            {
                validPosition = true;
            }
        }

        int size = sizeDist(gen);
        sf::Vector2f scale(size, size);

        auto rock = std::make_unique<obstacle>(
            "../Assets/Rock.png",
            pos,
            sf::Vector2f(0,0),
            sf::Vector2f(0,0),
            scale);

        // Add it to the collision manager
        game->collisionManager.AddStaticCollider(rock->GetBounds());

        game->obstacles.push_back(std::move(rock));

    }
}

void client_main::HandlePlayerDied(PlayerDiedMessage msg)
{
    if (!game)
        return;


}

void client_main::HandlePlayerRespawned(PlayerRespawnedMessage msg)
{
    if (!game)
        return;

    auto tank = game->tanks.find(msg.playerId);
    if (tank != game->tanks.end())
    {
        // Update tank position to respawn location
        tank->second->position = {msg.x, msg.y};

        // Clear bullets for this tank
        tank->second->bullets.clear();
        tank->second->Reset();

    }
}

void client_main::HandlePickUpData(PickUpMessage& msg)
{
    if (!game)
        return;

    Utils::printMsg("PickUps stuff received mate",debug);

    game->CreatePickups(msg);
}

void client_main::SendPickupHit(uint8_t pickupId, uint8_t pickupType)
{
    if (!isConnected)
        return;


    PickUpHitMessage hitMsg;
    hitMsg.playerId = playerId;
    hitMsg.pickUpId = pickupId;
    hitMsg.pickUpType = pickupType;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::PickUP_HIT) << hitMsg;

    socketUDP.send(packet, serverIp, serverPort);
}

void client_main::HandlePickUpUpdated(PickUpUpdatedMessage& msg)
{
    if (!game)
        return;

    // Find the pickup by its stored ID
    if (msg.pickUpType == 0)
    {
        // AmmoBox - search through ammoBoxes for matching ID
        for (auto& ammoBox : game->ammoBoxes)
        {
            if (ammoBox->GetPickupId() == msg.pickUpId)
            {
                ammoBox->SetPosition({msg.x, msg.y});
                ammoBox->SetActive(true);
                Utils::printMsg("AmmoBox repositioned to: " + std::to_string(msg.x) +
                               "," + std::to_string(msg.y), success);
                break;
            }
        }
    }
    else if (msg.pickUpType == 1)
    {
        // HealthKit - search through healthKits for matching ID
        for (auto& healthKit : game->healthKits)
        {
            if (healthKit->GetPickupId() == msg.pickUpId)
            {
                healthKit->SetPosition({msg.x, msg.y});
                healthKit->SetActive(true);
                Utils::printMsg("HealthKit repositioned to: " + std::to_string(msg.x) +
                               "," + std::to_string(msg.y), success);
                break;
            }
        }
    }
}