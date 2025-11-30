//
// Created by Pablo Gonzalez Poblette on 05/11/25.
//

#include "healthKit.h"

healthKit::healthKit(sf::Vector2f position,
                     float worldWidth,
                     float worldHeight,
                     int healAmount)
    : pickUp("../Assets/FirstAid.png", position, worldWidth, worldHeight),
      healAmount(healAmount)
{
    Utils::printMsg("HealthKit created with " + std::to_string(healAmount) + " heal amount", success);
}

bool healthKit::OnPickup()
{
    return true;
}