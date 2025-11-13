//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//

#pragma once
#include <SFML/Network/Packet.hpp>

// A simple tank update message
// FIXME: Consider what else we need to send and include it here.
struct TankMessage {

    float x, y;
    float rotationBody;
    float rotationBarrel;
    int playerId;


    friend sf::Packet& operator <<(sf::Packet& packet, const TankMessage& msg) {
        return packet << msg.x << msg.y
                      << msg.rotationBody << msg.rotationBarrel
                      << msg.playerId;
    }

    friend sf::Packet& operator >>(sf::Packet& packet, TankMessage& msg) {
        return packet >> msg.x >> msg.y
                      >> msg.rotationBody >> msg.rotationBarrel
                      >> msg.playerId;
    }
};