//
// Created by Pablo Gonzalez Poblette on 17/11/25.
//

#pragma once
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Window/Window.hpp>
#include <memory>
#include <SFML/Network/UdpSocket.hpp>

#include "../game/protocole_message.h"
#include "../game/game.h"

class client_main
{
    public:
        client_main(sf::IpAddress serverIp, unsigned short serverPort);

        void Update();

        // Connection Logic
        bool Connect();
        void Disconnect();

        // Input Logic
        void SendPosition();

        // Network Methods
        void ReceiveMessages();
        void HandleJoinAccepted(JoinAcceptedMessage msg);
        void HandleGameSnapShot(GameStateMessage msg);
        void HandlePlayerJoined(PlayerJoinedMessage msg);
        void HandlePlayerLeft(PlayerLeftMessage msg);
        void HandleBulletSpawned(BulletSpawnedMessage msg);
        void HandlePlayerHit(PlayerHitMessage msg);

        // Get Connection state and Player id
        bool IsConnected() const { return isConnected; }
        int GetPlayerId() const { return playerId; }

        // Game state
        std::unique_ptr<Game> game;

    private:
        // Network
        sf::UdpSocket socket_;
        sf::IpAddress serverIp;
        unsigned short serverPort;
        bool isConnected;

        // Player info
        int playerId;
        std::string playerColour;

        // Timing
        sf::Clock sendClock;
        const float SEND_RATE = 1.0f / 60.0f;  // 60 updates per second

        // Helper methods
        TankMessage TankPositionMessage();

};
