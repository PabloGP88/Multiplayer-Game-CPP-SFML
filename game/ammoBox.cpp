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
}
