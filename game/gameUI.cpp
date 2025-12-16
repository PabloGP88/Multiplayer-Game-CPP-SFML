//
// Created by Pablo Gonzalez Poblette on 04/11/25.
//

#include "gameUI.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <sstream>

gameUI::gameUI(const sf::Font& f)
    : healthText(f), ammoText(f)
{
    // Health text setup
    healthText.setCharacterSize(20);
    healthText.setFillColor(sf::Color::Green);
    healthText.setOutlineThickness(2.f);  // Outline thickness in pixels
    healthText.setOutlineColor(sf::Color::Black);  // Outline color

    // Ammo text setup
    ammoText.setCharacterSize(20);
    ammoText.setFillColor(sf::Color::Yellow);
    ammoText.setOutlineThickness(2.f);
    ammoText.setOutlineColor(sf::Color::Black);
}

void gameUI::Update(Tank& tank)
{
    // Convert health and ammo to strings
    std::stringstream ssHealth;
    ssHealth << "Health: " << tank.getHealth() << "%";

    std::stringstream ssAmmo;
    ssAmmo << "Ammo: " << tank.getAmmo();

    // Update text
    healthText.setString(ssHealth.str());
    ammoText.setString(ssAmmo.str());

    // Calculate health percentage
    float healthPercentage = static_cast<float>(tank.getHealth()) / static_cast<float>(tank.getMaxHealth());

    // Set health color based on percentage
    if (healthPercentage > 0.666f) // Above 2/3
    {
        healthText.setFillColor(sf::Color::Green);
    }
    else if (healthPercentage > 0.333f) // Above 1/3
    {
        healthText.setFillColor(sf::Color::Yellow);
    }
    else // Below 1/3
    {
        healthText.setFillColor(sf::Color::Red);
    }

    // Calculate ammo percentage
    float ammoPercentage = static_cast<float>(tank.getAmmo()) / static_cast<float>(tank.getMaxAmmo());

    // Set ammo color based on percentage
    if (ammoPercentage > 0.666f) // Above 2/3
    {
        ammoText.setFillColor(sf::Color::White);
    }
    else if (ammoPercentage > 0.333f) // Above 1/3
    {
        ammoText.setFillColor(sf::Color::Yellow);
    }
    else // Below 1/3
    {
        ammoText.setFillColor(sf::Color::Red);
    }
}

void gameUI::Draw(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();

    // Margin from the edges
    const float margin = 10.f;

    // Get bounds to align to bottom right
    sf::FloatRect ammoBounds = ammoText.getLocalBounds();
    sf::FloatRect healthBounds = healthText.getLocalBounds();

    // Position texts
    ammoText.setPosition(
{        windowSize.x - ammoBounds.size.x - margin,
        windowSize.y - ammoBounds.size.y - margin}
    );

    healthText.setPosition(
{        windowSize.x -  healthBounds.size.x - margin,
        ammoText.getPosition().y - healthBounds.size.y - 5.f }
    );

    // Draw on window
    window.draw(healthText);
    window.draw(ammoText);
}

