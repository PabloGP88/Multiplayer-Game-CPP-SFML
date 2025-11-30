//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//

#pragma once
#include <SFML/Network/Packet.hpp>
#include <vector>
#include <cstdint>

enum class MessageTypeProtocole : uint8_t {
    // Client -> Server
    JOIN_REQUEST = 0,
    TANK_UPDATE = 1,
    DISCONNECT = 2,
    PickUP_HIT = 3,

    // Server -> Client
    JOIN_ACCEPTED = 4,
    JOIN_REJECTED = 5,
    GAME_STATE = 6,
    PLAYER_JOINED = 7,
    PLAYER_LEFT = 8,
    BULLET_SPAWNED = 9,
    PLAYER_HIT = 10,
    PLAYER_DIED = 11,
    OBSTACLE_SEED = 12,
    PLAYER_RESPAWNED = 13,
    PickUP_DATA = 14,
    PickUp_UPDATE = 15
};

// Client requests to join
struct JoinRequestMessage {
    std::string playerName;

    friend sf::Packet& operator<<(sf::Packet& packet, const JoinRequestMessage& msg) {
        return packet << msg.playerName;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, JoinRequestMessage& msg) {
        return packet >> msg.playerName;
    }
};

// Server accepts join
struct JoinAcceptedMessage {
    uint8_t assignedPlayerId;
    std::string tankColor;

    friend sf::Packet& operator<<(sf::Packet& packet, const JoinAcceptedMessage& msg) {
        return packet << msg.assignedPlayerId << msg.tankColor;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, JoinAcceptedMessage& msg) {
        return packet >> msg.assignedPlayerId >> msg.tankColor;
    }
};

// Server rejects
struct JoinRejectedMessage {
    std::string message;

    friend sf::Packet& operator<<(sf::Packet& packet, const JoinRejectedMessage& msg) {
        return packet << msg.message;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, JoinRejectedMessage& msg) {
        return packet >> msg.message;
    }
};

// Client sends input every frame
struct TankMessage {
    float x, y;
    float rotationBody;
    float rotationBarrel;
    uint8_t playerId;
    bool shootPressed;
    bool isAlive;

    friend sf::Packet& operator<<(sf::Packet& packet, const TankMessage& msg) {
        return packet << msg.playerId
                      << msg.x << msg.y
                      << msg.rotationBody << msg.rotationBarrel
                      << msg.shootPressed << msg.isAlive;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, TankMessage& msg) {
        return packet >> msg.playerId
                      >> msg.x >> msg.y
                      >> msg.rotationBody >> msg.rotationBarrel
                      >> msg.shootPressed >> msg.isAlive;
    }
};

// Complete game state from server
struct GameStateMessage {
    struct PlayerState {
        uint8_t playerId;
        float x, y;
        float rotationBody;
        float rotationBarrel;
        uint8_t health;
        uint8_t ammo;
        bool isAlive;
        std::string color;
    };

    struct BulletState {
        uint8_t bulletId;
        float x, y;
        float rotation;
        uint8_t ownerId;
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
    uint8_t playerId;
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
    uint8_t playerId;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerLeftMessage& msg) {
        return packet << msg.playerId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerLeftMessage& msg) {
        return packet >> msg.playerId;
    }
};

// Server notifies bullet spawned
struct BulletSpawnedMessage {
    uint8_t bulletId;
    float x, y;
    float rotation;
    uint8_t ownerId;

    friend sf::Packet& operator<<(sf::Packet& packet, const BulletSpawnedMessage& msg) {
        return packet << msg.bulletId << msg.x << msg.y << msg.rotation << msg.ownerId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, BulletSpawnedMessage& msg) {
        return packet >> msg.bulletId >> msg.x >> msg.y >> msg.rotation >> msg.ownerId;
    }
};

// Server notifies player died
struct PlayerDiedMessage {
    uint8_t victimId;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerDiedMessage& msg) {
        return packet << msg.victimId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerDiedMessage& msg) {
        return packet >> msg.victimId;
    }
};

// Server notifies player respawned
struct PlayerRespawnedMessage {
    uint8_t playerId;
    float x, y;

    friend sf::Packet& operator<<(sf::Packet& packet, const PlayerRespawnedMessage& msg) {
        return packet << msg.playerId << msg.x << msg.y;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PlayerRespawnedMessage& msg) {
        return packet >> msg.playerId >> msg.x >> msg.y;
    }
};

struct ObstacleSeedMessage
{
    uint16_t seed;
    friend sf::Packet& operator<<(sf::Packet& packet, const ObstacleSeedMessage& msg)
    {
        return packet << msg.seed;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, ObstacleSeedMessage& msg)
    {
        return packet >> msg.seed;
    }
};

struct BulletDestroyedMessage
{
    uint8_t bulletId;

    friend sf::Packet& operator<<(sf::Packet& packet, const BulletDestroyedMessage& msg) {
        return packet << msg.bulletId;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, BulletDestroyedMessage& msg) {
        return packet >> msg.bulletId;
    }
};

struct PickUpMessage
{
    struct PickUpData
    {
        uint8_t pickUpId;
        uint8_t pickUpType;  // 0 = AmmoBox, 1 = HealthKit
        float x, y;
    };

    std::vector<PickUpData> pickUps;

    friend sf::Packet& operator<<(sf::Packet& packet, const PickUpMessage& msg)
    {
        packet <<  static_cast<uint8_t>(msg.pickUps.size());
        for (const auto& p : msg.pickUps)
        {
            packet << p.pickUpId << p.pickUpType << p.x << p.y;
        }

        return packet;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PickUpMessage& msg)
    {
        uint8_t count;
        packet >> count;
        msg.pickUps.clear();
        msg.pickUps.reserve(count);

        for (uint8_t i = 0; i < count; i++) {
            PickUpMessage::PickUpData p{};
            packet >> p.pickUpId >> p.pickUpType >> p.x >> p.y;
            msg.pickUps.push_back(p);
        }

        return packet;
    }
};

struct PickUpHitMessage
{
    uint8_t playerId;
    uint8_t pickUpId;
    uint8_t pickUpType;

    PickUpMessage pickUpMessage;

    friend sf::Packet& operator<<(sf::Packet& packet, const PickUpHitMessage& msg)
    {
        return packet << msg.playerId << msg.pickUpId << msg.pickUpType;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PickUpHitMessage& msg) {
        return packet >> msg.playerId >> msg.pickUpId >> msg.pickUpType;
    }
};

struct PickUpUpdatedMessage
{
    uint8_t pickUpId;
    uint8_t pickUpType;
    float x, y;

    friend sf::Packet& operator<<(sf::Packet& packet, const PickUpUpdatedMessage& msg)
    {
        return packet << msg.pickUpId << msg.pickUpType << msg.x << msg.y;
    }

    friend sf::Packet& operator>>(sf::Packet& packet, PickUpUpdatedMessage& msg)
    {
        return packet >> msg.pickUpId >> msg.pickUpType >> msg.x >> msg.y;
    }
};