//
// Created by Pablo Gonzalez Poblette on 03/11/25.
//

#include "decorations.h"
#include "utils.h"

decorations::decorations(float worldWidth, float worldHeight)
    : worldWidth(worldWidth), worldHeight(worldHeight)
{
    CreateSand();
    CreateFences();
}

void decorations::CreateSand()
{
    const float sandVectorWidth = 64.f;

    sandTexture.loadFromFile("Assets/tileSand1.png");

    sandTexture.setRepeated(true);

    // Top sand strip
    sf::Sprite topSand(sandTexture);
    topSand.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(worldWidth), static_cast<int>(sandVectorWidth)}));
    topSand.setPosition({0,0});
    sandVector.push_back(topSand);

    // Bottom sand strip
    sf::Sprite bottomSand(sandTexture);
    bottomSand.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(worldWidth), static_cast<int>(sandVectorWidth)}));
    bottomSand.setPosition({0, worldHeight - sandVectorWidth});
    sandVector.push_back(bottomSand);

    // Left sand strip
    sf::Sprite leftSand(sandTexture);
    leftSand.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(sandVectorWidth), static_cast<int>(worldHeight)}));
    leftSand.setPosition({0, 0});
    sandVector.push_back(leftSand);

    // Right sand strip
    sf::Sprite rightSand(sandTexture);
    rightSand.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(sandVectorWidth), static_cast<int>(worldHeight)}));
    rightSand.setPosition({worldWidth - sandVectorWidth, 0});
    sandVector.push_back(rightSand);
}

void decorations::CreateFences()
{
    const float fenceSize = 32.f;

    // Create horizontal fences (top and bottom)
    int numHorizontalFences = static_cast<int>(worldWidth / fenceSize);

    for (int i = 0; i < numHorizontalFences; ++i)
    {
        float x = i * fenceSize;

        // Top fence
        auto topFence = std::make_unique<obstacle>(
            "Assets/Horizontal Fence.png",
            sf::Vector2f(x + fenceSize/2, fenceSize/2),
            sf::Vector2f(0, 0),
            sf::Vector2f(0, 0),
            sf::Vector2f(1, 1)
        );
        fencesVector.push_back(std::move(topFence));

        // Bottom fence
        auto bottomFence = std::make_unique<obstacle>(
            "Assets/Horizontal Fence.png",
            sf::Vector2f(x + fenceSize/2, worldHeight - fenceSize/2),
            sf::Vector2f(0, 0),
            sf::Vector2f(0, 0),
            sf::Vector2f(1, 1)
        );
        fencesVector.push_back(std::move(bottomFence));
    }

    // Create vertical fences (left and right)
    int numVerticalFences = static_cast<int>(worldHeight / fenceSize);

    for (int i = 0; i < numVerticalFences; ++i)
    {
        float y = i * fenceSize;

        // Left fence
        auto leftFence = std::make_unique<obstacle>(
            "Assets/Vertical Fence.png",
            sf::Vector2f(fenceSize/2, y + fenceSize/2),
            sf::Vector2f(0, 0),
            sf::Vector2f(0, 0),
            sf::Vector2f(1, 1)
        );
        fencesVector.push_back(std::move(leftFence));

        // Right fence
        auto rightFence = std::make_unique<obstacle>(
            "Assets/Vertical Fence.png",
            sf::Vector2f(worldWidth - fenceSize/2, y + fenceSize/2),
            sf::Vector2f(0, 0),
            sf::Vector2f(0, 0),
            sf::Vector2f(1, 1)
        );
        fencesVector.push_back(std::move(rightFence));
    }
}

void decorations::Render(sf::RenderWindow& window)
{
    // Draw sand border strips
    for (const auto& sand : sandVector)
    {
        window.draw(sand);
    }

    // Draw fences
    for (const auto& fence : fencesVector)
    {
        fence->Render(window, false);
    }
}