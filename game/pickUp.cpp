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
    // Set a far distance for local so it looks responsive
    // later the server will actually sset the actual new position
    sf::Vector2f pos = {9999.f,9999.f};
    sprite.setPosition(pos);
    isActive = false;
}
