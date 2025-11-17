//
// Created by Pablo Gonzalez Poblette on 17/11/25.
//

#include "client_main.h"
#include "../game/utils.h"

client_main::client_main(sf::IpAddress serverIp, unsigned short serverPort)
    : serverIp(serverIp), serverPort(serverPort), isConnected(false), playerId(-1)
{
    socket_.setBlocking(false);
}

bool client_main::Connect()
{
    JoinRequestMessage joinMsg;
    joinMsg.playerName = "Player";

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::JOIN_REQUEST) << joinMsg;

    if (socket_.send(packet, serverIp, serverPort) != sf::Socket::Status::Done)
    {
        Utils::printMsg("Failed to send join request", error);
        return false;
    }

    Utils::printMsg("Join request sent, waiting for response...", info);

    // Wait for response
    sf::Clock timeout;
    while (timeout.getElapsedTime().asSeconds() < 5.0f)
    {
        sf::Packet responsePacket;
        std::optional<sf::IpAddress> sender;
        unsigned short port;

        if (socket_.receive(responsePacket, sender, port) == sf::Socket::Status::Done)
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
            }
        }

        sf::sleep(sf::milliseconds(100));
    }

    Utils::printMsg("Connection timeout", error);
    return false;
}

void client_main::Disconnect()
{
    if (!isConnected)
        return;

    sf::Packet packet;
    packet << static_cast<uint8_t>(MessageTypeProtocole::DISCONNECT) << playerId;
    socket_.send(packet, serverIp, serverPort);

    isConnected = false;
    Utils::printMsg("Disconnected from server", warning);
}

void client_main::Update()
{
    ReceiveMessages();
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

    socket_.send(packet, serverIp, serverPort);
}

void client_main::ReceiveMessages()
{
    if (!isConnected)
        return;

    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while (socket_.receive(packet, sender, port) == sf::Socket::Status::Done)
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

            case MessageTypeProtocole::PLAYER_HIT:
            {
                PlayerHitMessage msg;
                if (packet >> msg)
                {
                    HandlePlayerHit(msg);
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

    Utils::printMsg("Connected! Player ID: " + std::to_string(playerId) +
                   " Color: " + playerColour, success);
}

void client_main::HandleGameSnapShot(GameStateMessage msg)
{
    if (!game)
        return;

    for (const auto& playerState : msg.players)
    {
        // Don't overwrite local player from server
        if (playerState.playerId == playerId)
            continue;

        // Add tank if doesn't exist
        if (game->tanks.find(playerState.playerId) == game->tanks.end())
        {
            game->AddTank(playerState.playerId, playerState.color);
        }

        // Update remote tank
        Tank* tank = game->tanks[playerState.playerId].get();
        tank->position = {playerState.x, playerState.y};
        tank->bodyRotation = sf::degrees(playerState.rotationBody);
        tank->barrelRotation = sf::degrees(playerState.rotationBarrel);
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

        Utils::printMsg("Bullet spawned by player " + std::to_string(msg.ownerId) +
                       " at (" + std::to_string(msg.x) + ", " + std::to_string(msg.y) + ")",
                       debug);
    }
}

void client_main::HandlePlayerHit(PlayerHitMessage msg)
{
    if (!game)
        return;

    Utils::printMsg("Player " + std::to_string(msg.victimId) + " was hit for " +
                   std::to_string(msg.damage) + " damage", warning);
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

    // Check if space key is pressed
    msg.shootPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    return msg;
}