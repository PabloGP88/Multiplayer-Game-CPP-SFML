//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//

#pragma once
#include <SFML/Network/Packet.hpp>
#include <vector>
#include <cstdint>

// Message types
enum class MessageTypeProtocole : uint8_t {
    // Client -> Server
    JOIN_REQUEST = 0,
    TANK_UPDATE = 1,
    DISCONNECT = 2,

    // Server -> Client
    JOIN_ACCEPTED = 3,
    JOIN_REJECTED = 4,
    GAME_STATE = 5,
    PLAYER_JOINED = 6,
    PLAYER_LEFT = 7,
    BULLET_SPAWNED = 8,
    PLAYER_HIT = 9,
    PLAYER_DIED = 10,
    OBSTACLE_DATA = 11,
};

// Client requests to join
struct JoinRequestMessage {
    std::string playerName;  // Optional for now

    friend sf::Packet& operator<<(sf::Packet& packet, const JoinRequestMessage& msg) {
        return packet << msg.playerName;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, JoinRequestMessage& msg) {
        return packet >> msg.playerName;
    }
};

// Server accepts join
struct JoinAcceptedMessage {
    int assignedPlayerId;
    std::string tankColor;

    friend sf::Packet& operator<<(sf::Packet& packet, const JoinAcceptedMessage& msg) {
        return packet << msg.assignedPlayerId << msg.tankColor;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, JoinAcceptedMessage& msg) {
        return packet >> msg.assignedPlayerId >> msg.tankColor;
    }
};

// Client sends input every frame
struct TankMessage {
    float x, y;
    float rotationBody;
    float rotationBarrel;
    int playerId;
    bool shootPressed;

    friend sf::Packet& operator<<(sf::Packet& packet, const TankMessage& msg) {
        return packet << msg.playerId
                      << msg.x << msg.y
                      << msg.rotationBody << msg.rotationBarrel
                      << msg.shootPressed;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, TankMessage& msg) {
        return packet >> msg.playerId
                      >> msg.x >> msg.y
                      >> msg.rotationBody >> msg.rotationBarrel
                      >> msg.shootPressed;
    }
};

// Complete game state from server
struct GameStateMessage {
    struct PlayerState {
        int playerId;
        float x, y;
        float rotationBody;
        float rotationBarrel;
        int health;
        int ammo;
        bool isAlive;
        std::string color;
    };

    struct BulletState {
        int bulletId;
        float x, y;
        float rotation;
        int ownerId;
    };

    std::vector<PlayerState> players;
    std::vector<BulletState> bullets;

    friend sf::Packet& operator<<(sf::Packet& packet, const GameStateMessage& msg) {
        // Players
        packet << static_cast<uint32_t>(msg.players.size());
        for (const auto& p : msg.players) {
            packet << p.playerId << p.x << p.y
                   << p.rotationBody << p.rotationBarrel
                   << p.health << p.ammo << p.isAlive << p.color;
        }

        // Bullets
        packet << static_cast<uint32_t>(msg.bullets.size());
        for (const auto& b : msg.bullets) {
            packet << b.bulletId << b.x << b.y << b.rotation << b.ownerId;
        }
        return packet;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, GameStateMessage& msg) {
        uint32_t playerCount, bulletCount;

        // Players
        packet >> playerCount;
        msg.players.clear();
        msg.players.reserve(playerCount);
        for (uint32_t i = 0; i < playerCount; i++) {
            GameStateMessage::PlayerState p;
            packet >> p.playerId >> p.x >> p.y
                   >> p.rotationBody >> p.rotationBarrel
                   >> p.health >> p.ammo >> p.isAlive >> p.color;
            msg.players.push_back(p);
        }

        // Bullets
        packet >> bulletCount;
        msg.bullets.clear();
        msg.bullets.reserve(bulletCount);
        for (uint32_t i = 0; i < bulletCount; i++) {
            GameStateMessage::BulletState b;
            packet >> b.bulletId >> b.x >> b.y >> b.rotation >> b.ownerId;
            msg.bullets.push_back(b);
        }
        return packet;
    }
};

// Server notifies new player joined
struct PlayerJoinedMessage {
    int playerId;
    std::string color;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerJoinedMessage& msg) {
        return packet << msg.playerId << msg.color;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerJoinedMessage& msg) {
        return packet >> msg.playerId >> msg.color;
    }
};

// Server notifies player left
struct PlayerLeftMessage {
    int playerId;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerLeftMessage& msg) {
        return packet << msg.playerId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerLeftMessage& msg) {
        return packet >> msg.playerId;
    }
};

// Server notifies bullet spawned
struct BulletSpawnedMessage {
    int bulletId;
    float x, y;
    float rotation;
    int ownerId;

    friend sf::Packet& operator<<(sf::Packet& packet, const BulletSpawnedMessage& msg) {
        return packet << msg.bulletId << msg.x << msg.y << msg.rotation << msg.ownerId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, BulletSpawnedMessage& msg) {
        return packet >> msg.bulletId >> msg.x >> msg.y >> msg.rotation >> msg.ownerId;
    }
};

// Server notifies player hit
struct PlayerHitMessage {
    int victimId;
    int shooterId;
    int damage;
    int newHealth;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerHitMessage& msg) {
        return packet << msg.victimId << msg.shooterId << msg.damage << msg.newHealth;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerHitMessage& msg) {
        return packet >> msg.victimId >> msg.shooterId >> msg.damage >> msg.newHealth;
    }
};

struct ObstacleSpawnedMessage
{
    struct ObstacleData
    {
        std::string texture;
        float x, y;
        float width, height;
        float scaleX, scaleY;
    };

    std::vector<ObstacleData> obstacles;

    friend sf::Packet& operator<<(sf::Packet& packet, const ObstacleSpawnedMessage& msg)
    {
        packet << static_cast<uint32_t>(msg.obstacles.size());
        for (const auto& p : msg.obstacles)
        {
            packet << p.x << p.y << p.width << p.height << p.scaleX << p.scaleY << p.texture;
        }
        return packet;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, ObstacleSpawnedMessage& msg)
    {
        uint32_t count;
        packet >> count;
        msg.obstacles.clear();
        msg.obstacles.reserve(count);

        for (uint32_t i = 0; i < count; i++) {
            ObstacleSpawnedMessage::ObstacleData obs;
            packet >> obs.x >> obs.y >> obs.width >> obs.height
                   >> obs.scaleX >> obs.scaleY >> obs.texture;

            msg.obstacles.push_back(obs);
        }
        return packet;
    }
};