//
// Created by Pablo Gonzalez Poblette on 05/11/25.
//

#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include <string>
#include "utils.h"

class pickUp
{public:
        pickUp(const std::string& texturePath,
               sf::Vector2f position,
               float worldWidth = 1280.f,
               float worldHeight = 960.f);

        virtual ~pickUp() = default;

        void Render(sf::RenderWindow& window) const;

        // Check collision with a rectangle (tank bounds)
        bool CheckCollision(const sf::FloatRect& rect) const;

        // Respawn at a random location within boundaries
        void Respawn();

        // Get bounds for collision detection
        sf::FloatRect GetBounds() const;

        sf::Vector2f GetPosition() const { return position; }

        bool IsActive() const { return isActive; }

        void SetActive(bool active) { isActive = active; }
        void SetPickupId(uint8_t id) { pickUpdId = id; }
        uint8_t GetPickupId() const { return pickUpdId; }


    protected:
        sf::Vector2f position;
        sf::Texture texture;
        sf::Sprite sprite;

        float worldWidth;
        float worldHeight;

        bool isActive;

        // Random number generation for respawn
        static std::random_device rd;
        static std::mt19937 gen;

        uint8_t pickUpdId;

};