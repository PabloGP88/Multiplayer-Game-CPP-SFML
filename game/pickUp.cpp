//
// Created for tank game pickup system
//
#include "pickUp.h"

// Initialize static members
std::random_device pickUp::rd;
std::mt19937 pickUp::gen(rd());

pickUp::pickUp(const std::string& texturePath,
               sf::Vector2f position,
               float worldWidth,
               float worldHeight)
    : position(position), sprite(texture),
      worldWidth(worldWidth),
      worldHeight(worldHeight),
      isActive(true)
{
    if (!texture.loadFromFile(texturePath))
    {
        Utils::printMsg("Could not load texture: " + texturePath, warning);
    }

    // Setup sprite
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, (sf::Vector2i)texture.getSize()));
    sprite.setOrigin((sf::Vector2f)sprite.getTextureRect().getCenter());
    sprite.setPosition(position);
    sprite.setScale({2.f,2.f});
}

void pickUp::Update(float dt)
{
    // Be implemented in ammoBox and health
}

void pickUp::Render(sf::RenderWindow& window) const
{
    if (isActive)
    {
        window.draw(sprite);
    }
}

bool pickUp::CheckCollision(const sf::FloatRect& rect) const
{
    if (!isActive)
        return false;

    return GetBounds().findIntersection(rect).has_value();
}

sf::FloatRect pickUp::GetBounds() const
{
    return sprite.getGlobalBounds();
}

void pickUp::Respawn()
{
    position = GenerateRandomPosition();
    sprite.setPosition(position);
    isActive = true;

    Utils::printMsg("Pickup respawned at: " +
                    std::to_string(position.x) + ", " +
                    std::to_string(position.y), success);
}

sf::Vector2f pickUp::GenerateRandomPosition()
{
    // Create distributions for x and y within valid boundaries
    std::uniform_real_distribution<float> distX(
        BORDER_MARGIN,
        worldWidth - BORDER_MARGIN
    );
    std::uniform_real_distribution<float> distY(
        BORDER_MARGIN,
        worldHeight - BORDER_MARGIN
    );

    return sf::Vector2f(distX(gen), distY(gen));
}