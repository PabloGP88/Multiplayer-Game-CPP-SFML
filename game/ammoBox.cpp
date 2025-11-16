//
// Created by Pablo Gonzalez Poblette on 05/11/25.
//

#include "ammoBox.h"

ammoBox::ammoBox(sf::Vector2f position,
                 float worldWidth,
                 float worldHeight,
                 int ammoAmount)
    : pickUp("Assets/AmmoBox.png", position, worldWidth, worldHeight),
      ammoAmount(ammoAmount)
{
    Utils::printMsg("AmmoBox created with " + std::to_string(ammoAmount) + " ammo", success);
}

bool ammoBox::OnPickup()
{
    // Return true to indicate this pickup should be consumed
    Utils::printMsg("AmmoBox picked up! Providing " + std::to_string(ammoAmount) + " ammo", success);
    return true;
}