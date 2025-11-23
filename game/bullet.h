//
// Created by Pablo Gonzalez Poblette on 13/10/25.
//

#pragma once
#include <SFML/Graphics.hpp>

class CollisionManager;
class Tank;

class bullet
{
public:
    bullet(sf::Vector2f startPosition, sf::Angle direction, float speed = 800.f, int damage = 10);

    void Update(float dt, CollisionManager& collisionManager);
    void Render(sf::RenderWindow& window);

    // Check if bullet is still active
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }

    // Get bullet position for collision detection
    sf::Vector2f GetPosition() const { return position; }
    sf::FloatRect GetBounds() const { return sprite.getGlobalBounds(); }

    // Check collision with tank
    bool CheckTankCollision(Tank* tank);

    // Get damage amount
    int GetDamage() const { return damage; }

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Angle rotation;
    float speed;
    bool isActive;
    int damage;

    sf::Texture texture;
    sf::Sprite sprite = sf::Sprite(texture);

    // World boundaries
    const float WORLD_WIDTH = 1280.f;
    const float WORLD_HEIGHT = 960.f;
};
