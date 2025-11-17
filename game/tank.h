//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "protocole_message.h"
#include "bullet.h"
#include "pickUp.h"

class CollisionManager;

class Tank
{
public:
    // Colour string will be used in path for image texture loading.
    // Will work with "red", "blue", "green" and "black".
    //
    // FIXME: this is not the cleanest solution as you can make a typo which will cause
    // texture to fail to load. Ideally should use enum/map or similar solution.
    explicit Tank(std::string colour);

    void Update(float dt, CollisionManager& collisionManager);
    const void Render(sf::RenderWindow &window);

    // Shooting functionalities, wE NEED METHOD TO SHOOT, RENDER DE BULLETS AND UPDATE THEIR POSITION FOR EACH TANK
    void Shoot();
    void UpdateBullets(float dt, CollisionManager& collisionManager);
    void RenderBullets(sf::RenderWindow& window);

    sf::Vector2f position = {0.f, 0.f};
    sf::Angle barrelRotation = sf::degrees(0);
    sf::Angle bodyRotation = sf::degrees(0);

    // Get collision bounds for the tank
    sf::FloatRect GetBounds() const;


    struct {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
    } isMoving;

    struct
    {
        bool right = false;
        bool left = false;
    } isAiming;

    // Pickup and damage system
    void CheckPickupCollision(pickUp* pickup);
    void TakeDamage(int damage);

    void AddAmmo(int amount);
    void AddHealth(int amount);

    int getAmmo();
    int getHealth();

    int getMaxHealth();
    int getMaxAmmo();

    bool IsAlive() const { return health > 0; }

    std::string GetColor() const { return colorString; }

    // Bullet management
    std::vector<std::unique_ptr<bullet>> bullets;

private:
    // Temporary placeholder texture, make sue to replace before rendering the sprite.
    sf::Texture placeholder = sf::Texture(sf::Vector2u(1, 1));

    sf::Texture bodyTexture;
    sf::Texture barrelTexture;

    // These can (and probably should) be replaced with std::optional or unique pointers,
    // to remove the need to use placeholder textures for sprite initialisation.
    sf::Sprite body = sf::Sprite(placeholder);
    sf::Sprite barrel = sf::Sprite(placeholder);

    float movementSpeed = 150.f;
    float rotationSpeed = 200.f;
    float barrelSpeed = 300.0f;

    // Saving current colour here in case we need to send elsewhere.
    std::string colorString;

    float barrelLength = 30.f; // Distance from tank center to barrel tip

    const int maxHealth = 100;
    const int maxAmmo = 20;

    int health = 50;
    int ammo = 20;
};