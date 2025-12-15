#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "ammoBox.h"
#include "collision_manager.h"
#include "decorations.h"
#include "gameUI.h"
#include "healthKit.h"
#include "obstacle.h"
#include "tank.h"
#include "random"

class Game
{
public:
    explicit Game(int localPlayer);

    void HandleEvents(std::optional<sf::Event> event, int tankId);
    void Update(float dt);
    void NetworkUpdate(float dt, int idTank, TankMessage data);
    void Render(sf::RenderWindow &window);
    TankMessage GetNetworkUpdate(int id);

    void AddTank(int tankId, std::string tankColour);

    void CreatePickups(PickUpMessage& msg);
    int localId;
    std::unordered_map<int, std::unique_ptr<Tank>> tanks;

    CollisionManager collisionManager;

    // Vector of obstacles to store them and render them all in a foreach loop
    std::vector<std::unique_ptr<obstacle>> obstacles;


    // Bullet boxes and Health
    std::vector<std::unique_ptr<ammoBox>> ammoBoxes;
    std::vector<std::unique_ptr<healthKit>> healthKits;

    std::function<void(uint8_t pickupId, uint8_t pickupType)> OnPickupCollected;

    // Interpolation logic

    struct RemoteTankData
    {
        sf::Vector2f position;
        sf::Angle bodyRotation;
        sf::Angle barrelRotation;
        sf::Time timestamp;
    };

    std::unordered_map<int, RemoteTankData> previousStates;
    std::unordered_map<int, RemoteTankData> targetStates;
    std::unordered_map<int, sf::Clock> interpClocks;

    // this is the interpolation window in seconds
    const float INTERP_TIME = 0.2f;

    void AddNetworkTankState(int tankId, const GameStateMessage::PlayerState& state);

private:

    sf::View camera; // Camera for the game

    // Temporary placeholder texture, make sue to replace before rendering the sprite.
    sf::Texture placeholder = sf::Texture(sf::Vector2u(1, 1));
    sf::Texture backgroundTexture;

    void UpdatePickups(float dt);
    void RenderPickups(sf::RenderWindow& window);

    decorations decoration;

    // This can (and probably should) be replaced with std::optional or a unique pointer,
    // to remove the need to use placeholder textures for sprite initialisation.
    sf::Sprite background = sf::Sprite(placeholder);
    bool receivedColour = false;

    void CreateObstacles();

    sf::Font uiFont;

    gameUI ui;

    const float WORLD_WIDTH = 1280.f;
    const float WORLD_HEIGHT = 960.f;

    // Interpolation
    void InterpolateRemoteTanks(CollisionManager& collisionManager, float dt, int tankID);

    sf::Angle findLerpAngle(sf::Angle angle1, sf::Angle angle2, float t);


};