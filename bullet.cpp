//
// Created by Pablo Gonzalez Poblette on 13/10/25.
//
#include "bullet.h"
#include "utils.h"
#include "Tank.h"
#include <cmath>
#include "collision_manager.h"

bullet::bullet(sf::Vector2f startPosition, sf::Angle direction, float speed, int damage)
    : position(startPosition), rotation(direction), speed(speed), isActive(true), damage(damage)
{
    // Load bullet texture
    if (!texture.loadFromFile("Assets/bullet.png"))
    {
        Utils::printMsg("Could not load texture: Assets/bullet.png", warning);
    }

    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, (sf::Vector2i)texture.getSize()));
    sprite.setOrigin((sf::Vector2f)sprite.getTextureRect().getCenter());
    sprite.setPosition(position);
    sprite.setRotation(rotation);

    // Calculate velocity based on direction
    // Add 90 degrees because sprite faces right by default
    float angleRadians = (rotation + sf::degrees(90)).asRadians();
    velocity = {
        std::cos(angleRadians) * speed,
        std::sin(angleRadians) * speed
    };
}

void bullet::Update(float dt, CollisionManager& collisionManager)
{
    if (!isActive) return;

    position += velocity * dt;
    sprite.setPosition(position);

    // Check for collisions with static objects
    sf::Vector2f pushback;
    sf::FloatRect bounds = GetBounds();

    if (collisionManager.CheckCollision(bounds, pushback))
    {
        Utils::printMsg("Bullet collided with obstacle, destroying bullet", success);
        isActive = false;
    }
}

bool bullet::CheckTankCollision(Tank* tank)
{
    if (!isActive || !tank || !tank->IsAlive())
        return false;

    sf::FloatRect bulletBounds = GetBounds();
    sf::FloatRect tankBounds = tank->GetBounds();

    // Check if bullet intersects with tank
    if (bulletBounds.findIntersection(tankBounds).has_value())
    {
        Utils::printMsg("Bullet hit tank!", warning);
        tank->TakeDamage(damage);
        isActive = false;
        return true;
    }

    return false;
}

void bullet::Render(sf::RenderWindow& window)
{
    if (isActive)
    {
        window.draw(sprite);
    }
}