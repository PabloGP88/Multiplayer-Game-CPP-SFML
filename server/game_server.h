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

    std::string playerName;

    sf::Clock lastHeartbeat;  // For timeout detection
    bool prevShootState = false;  // Track previous shoot state for edge detection

    ConnectedClient(sf::IpAddress address, unsigned short port, int playerId, std::string playerName)
    : ipAddress(address), port(port), playerId(playerId), playerName(playerName),prevShootState(false) {}
};

struct Bullet {
    int bulletId;
    std::unique_ptr<bullet> bulletPrefab;
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

        void Update();

    private:
        // Networking

        sf::TcpListener listenerTCP;
        sf::SocketSelector selectorTCP;
        std::vector<sf::TcpSocket*> clientsTCP;

        sf::UdpSocket socketUDP;
        std::unordered_map<int, ConnectedClient> clientsUDP;

        // Game state
        std::unordered_map<int, std::unique_ptr<Tank>> tanks;
        std::vector<Bullet> bullets;
        CollisionManager collisionManager;

        // ID management
        int nextPlayerId = 0;
        int nextBulletId = 0;

        // possible colors
        std::vector<std::string> availableColors = {"blue", "red", "green", "black"};
        std::vector<bool> colorUsed = {false, false, false, false};

        // Server settings
        const float TICK_RATE = 60.0f;  // ticks per second, based on how valve has tickrate for csgo https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking
        const float CLIENT_TIMEOUT = 10.0f;  // seconds timeout
        const float RESPAWN_TIME = 2.0f; // 2 seconds
        const int SLEEP_TIME = 10; // miliseconds

        // Obstacles
        std::vector<std::unique_ptr<obstacle>> obstacles;

        // PickUps
        std::vector<std::unique_ptr<ammoBox>> ammoBoxes;
        std::vector<std::unique_ptr<healthKit>> healthKits;

        std::vector<std::unique_ptr<pickUp>> pickUps;

        // array to store respowning tanks
        std::vector<RespawnClient> pendingRespawns;


        // Methods
        void ProcessMessages();
        void ProcessMessagesTCP(sf::TcpSocket& socket, MessageTypeProtocole type, sf::Packet& packet);
        void SendGameSnapShot();
        void CheckClientTimeouts();

        void BroadcastMessageTCP(sf::Packet& packet);

        void SendObstacleSeedTCP(sf::TcpSocket& socket);

        void SendPickUpsPositionTCP(sf::TcpSocket& socket);

        void HandleJoinRequestTCP(sf::TcpSocket& ,JoinRequestMessage msg);
        void HandleTankUpdate(TankMessage msg);
        void HandlePickUpsUpdate(PickUpHitMessage msg);
        void HandleDisconnect(int playerId);

        void SpawnBullet(int ownerId);

        void CreatePickUps();

        void BroadcastMessage(sf::Packet& packet);
        void SendToClient(int playerId, sf::Packet& packet);

        void CheckPendingRespawns();
        void RespawnPlayer(int playerId);

        std::string AssignColor();
        void FreeColor(const std::string& color);

        const float ROCK_SPACE = 64.0;

        uint16_t SEED;

        GameStateMessage BuildGameState();
};