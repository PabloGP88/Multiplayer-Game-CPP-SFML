//
// Created by Pablo Gonzalez Poblette on 16/11/25.
//

#pragma once
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <unordered_map>
#include <memory>
#include "../game/collision_manager.h"
#include "../game/tank.h"
#include "../game/protocole_message.h"


struct ConnectedClient {
    sf::IpAddress ipAddress;
    unsigned short port;
    int playerId;

    sf::Clock lastHeartbeat;  // For timeout detection
};

struct ServerBullet {
    int bulletId;
    std::unique_ptr<bullet> bulletObj;
    int ownerId;
};

class game_server
{
    public:
        game_server(unsigned short port);

        void Update();  // Main server loop

    private:
        // Networking
        sf::UdpSocket socket;
        std::unordered_map<int, ConnectedClient> clients;

        // Game state
        std::unordered_map<int, std::unique_ptr<Tank>> tanks;
        std::vector<ServerBullet> bullets;
        CollisionManager collisionManager;

        // ID management
        int nextPlayerId = 0;
        int nextBulletId = 0;

        // possible colors
        std::vector<std::string> availableColors = {"blue", "red", "green", "black"};
        std::vector<bool> colorUsed = {false, false, false, false};

        // Server settings
        const float TICK_RATE = 60.0f;  // 60 ticks per second
        const float CLIENT_TIMEOUT = 5.0f;  // 5 seconds timeout

        // Methods
        void ProcessMessages();
        void UpdateSnapShot(float dt);
        void SendGameSnapShot();
        void CheckClientTimeouts();

        void HandleJoinRequest(sf::IpAddress sender, unsigned short port, JoinRequestMessage msg);
        void HandleClientInput(ClientInputMessage msg);
        void HandleDisconnect(int playerId);

        void SpawnBullet(int ownerId);
        void UpdateBullets(float dt);
        void CheckBulletCollisions();

        void BroadcastMessage(sf::Packet& packet);
        void SendToClient(int playerId, sf::Packet& packet);

        std::string AssignColor();
        void FreeColor(const std::string& color);

        GameStateMessage BuildGameState();
};

