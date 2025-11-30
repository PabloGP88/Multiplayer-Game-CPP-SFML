//
// Created by Pablo Gonzalez Poblette on 05/11/25.
//

#ifndef TANK_GAME_AMMOBOX_H
#define TANK_GAME_AMMOBOX_H
#include "pickUp.h"


class ammoBox: public pickUp
{
    public:
        ammoBox(sf::Vector2f position,
                float worldWidth = 1280.f,
                float worldHeight = 960.f,
                int ammoAmount = 5);


        bool OnPickup() override;

        // Get the amount of ammo this box provides
        int GetAmmoAmount() const { return ammoAmount; }

        void SetPosition(sf::Vector2f position)
        {
            sprite.setPosition(position);
            this->position = position;
        };
    private:
        int ammoAmount;

};


#endif //TANK_GAME_AMMOBOX_H