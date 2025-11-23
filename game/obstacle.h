//
// Created by Pablo Gonzalez Poblette on 03/11/25.
//

#pragma once
#include "collision_manager.h"
#include "utils.h"

class obstacle
{
public:
    obstacle(const std::string& texturePath,
             sf::Vector2f pos,
             sf::Vector2f collSize = sf::Vector2f(0, 0),
             sf::Vector2f collOffset = sf::Vector2f(0, 0),
             sf::Vector2f scale = sf::Vector2f(1, 1));

    void Render(sf::RenderWindow& window, bool debugMode = false) const;

    sf::FloatRect GetBounds() const;

    sf::Sprite sprite;

    sf::Vector2f GetPosition() { return position;}
    sf::Vector2f GetScale() { return scale;}

    std::string GetTexturePath() const { return  texturePath;}

private:
    sf::Texture texture;
    std::string texturePath;

    sf::Vector2f position;
    sf::Vector2f colliderSize;
    sf::Vector2f colliderOffset;
    sf::Vector2f scale;

    sf::Color debugColor;

    sf::RectangleShape debugRect;
    void UpdateDebugRect();
};
