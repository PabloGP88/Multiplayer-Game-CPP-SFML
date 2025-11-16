//
// Created by Pablo Gonzalez Poblette on 03/11/25.
//

#include "../obstacle.h"

obstacle::obstacle(const std::string& texturePath,
                   sf::Vector2f position,
                   sf::Vector2f colliderSize,
                   sf::Vector2f colliderOffset,
                   sf::Vector2f scale)
    : sprite(texture),
      position(position),
      colliderSize(colliderSize),
      colliderOffset(colliderOffset),
      scale(scale),
      debugColor(255, 0, 0, 255)
{

    if (texture.loadFromFile(texturePath))
    {
        Utils::printMsg("Texture Loaded Successfully: " + texturePath, success);

        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2i>(texture.getSize())));

        sprite.setOrigin(static_cast<sf::Vector2f>(sprite.getTextureRect().getCenter()));
        sprite.setPosition(position);
        sprite.setScale(scale);

        if (this->colliderSize.x == 0 || this->colliderSize.y == 0)
        {
            sf::FloatRect spriteBounds = sprite.getGlobalBounds();
            this->colliderSize = spriteBounds.size;
        }

    } else
    {
        Utils::printMsg("Texture Loading Failed", error);
    }

    UpdateDebugRect();

}

sf::FloatRect obstacle::GetBounds() const
{
    sf::Vector2f colliderPosition = position + colliderOffset - (colliderSize / 2.f);
    return {colliderPosition, colliderSize};
}

void obstacle::Render(sf::RenderWindow& window, const bool debugMode) const
{
    window.draw(sprite);

    if (debugMode)
    {
        window.draw(debugRect);
    }
}

void obstacle::UpdateDebugRect()
{
    debugRect.setSize(colliderSize);
    debugRect.setOrigin(sf::Vector2f(0, 0));
    debugRect.setPosition(position + colliderOffset - (colliderSize / 2.f));
    debugRect.setFillColor({0,0,0,0});
    debugRect.setOutlineColor(sf::Color(debugColor.r, debugColor.g, debugColor.b, 255));
    debugRect.setOutlineThickness(3);
}