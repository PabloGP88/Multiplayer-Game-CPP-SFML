//
// Created by Pablo Gonzalez Poblette on 04/11/25.
//
#pragma once

#include <string>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include "tank.h"

class gameUI

{
public:

    gameUI(const sf::Font& f);
    bool loadFont(const std::string& fontPath);

    void Update(Tank& tank);
    void Draw(sf::RenderWindow& window);
    void SetPosition(sf::Vector2f pos);

private:

    sf::Font font;
    sf::Text healthText;
    sf::Text ammoText;
};
