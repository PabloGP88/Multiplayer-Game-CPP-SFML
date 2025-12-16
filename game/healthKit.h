//
// Created by Pablo Gonzalez Poblette on 05/11/25.
//

#pragma once


#include "pickUp.h"

class healthKit: public pickUp
{
    public:
        healthKit(sf::Vector2f position,
                  float worldWidth = 1280.f,
                  float worldHeight = 960.f,
                  int healAmount = 25);

        // Get the amount of health this kit provides
        int GetHealAmount() const { return healAmount; }

        void SetPosition(sf::Vector2f position)
            {
                this->position = position;
                sprite.setPosition(position);
            };

    private:
        int healAmount;
};

