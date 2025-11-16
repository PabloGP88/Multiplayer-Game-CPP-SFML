//
// Created by Pablo Gonzalez Poblette on 03/11/25.
//
#pragma once

#include <SFML/Graphics.hpp>
#include "obstacle.h"


class decorations
{
    public:
    decorations(float worldWidth, float worldHeight);

    void Render(sf::RenderWindow& window);

    private:
    float worldWidth;
    float worldHeight;

    sf::Texture sandTexture;
    std::vector<sf::Sprite> sandVector;

    std::vector<std::unique_ptr<obstacle>> fencesVector;


    void CreateFences();
    void CreateSand();

};
