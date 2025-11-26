//
// Created by Pablo Gonzalez Poblette on 16/11/25.
//

#pragma once
#include <SFML/Network.hpp>
#include <unordered_map>
#include <memory>

#include "../game/ammoBox.h"
#include "../game/collision_manager.h"
#include "../game/healthKit.h"
#include "../game/obstacle.h"
#include "../game/tank.h"
#include "../game/protocole_message.h"


struct ConnectedClient {
    sf::IpAddress ipAddress;
    unsigned short port;
    int playerId;
    bool isPendingRespawn;

    sf::Clock lastHeartbeat;  // For timeout detection
    bool prevShootState = false;  // Track previous shoot state for edge detection

    ConnectedClient(sf::IpAddress address, unsigned short port, int playerId)
    : ipAddress(address), port(port), playerId(playerId), prevShootState(false) {}
};

struct ServerBullet {
    int bulletId;
    std::unique_ptr<bullet> bulletObj;
    int ownerId;
};

struct RespawnClient
{
    int victimId;
    int killerId;
    sf::Clock deathTimer;

    RespawnClient(int pId, int kId) : victimId(pId), killerId(kId) {
        deathTimer.restart();
    }
};


class game_server
{
    public:
        explicit game_server(unsigned short port);

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
        const float RESPAWN_TIME = 2.0f; // 2 seconds

        // Obstacles
        std::vector<std::unique_ptr<obstacle>> obstacles;

        // PickUps
        std::vector<std::unique_ptr<ammoBox>> ammoBoxes;
        std::vector<std::unique_ptr<healthKit>> healthKits;

        // Death and respawn tracking
        std::vector<RespawnClient> pendingRespawns;


        // Methods
        void ProcessMessages();
       // void UpdateSnapShot(float dt);
        void SendGameSnapShot();
        void CheckClientTimeouts();

        void SendObstaclesPosition(int playerId);
        void SendPickUpsPosition(int playerId);
        void HandleJoinRequest(sf::IpAddress sender, unsigned short port, JoinRequestMessage msg);
        void HandleTankUpdate(TankMessage msg);
        void HandleDisconnect(int playerId);

        void SpawnBullet(int ownerId);

        void CreateObstacles();
        void CreatePowerUps();

        void BroadcastMessage(sf::Packet& packet);
        void SendToClient(int playerId, sf::Packet& packet);

        void CheckPendingRespawns();
        void RespawnPlayer(int playerId);

        std::string AssignColor();
        void FreeColor(const std::string& color);

        GameStateMessage BuildGameState();
};
