//
// Created by Pablo Gonzalez Poblette on 28/10/25.
//
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// I created  this script based on the collision system AABB with help of this YouTube tutorial and AI Claude
// https://www.youtube.com/watch?v=IaUcAt0jDqs&t=607s

struct CollisionBox {
    sf::FloatRect bounds;     // The rectangle (x, y, width, height)
    bool isStatic;            // Static = doesn't move (walls), Dynamic = moves (tanks)
    int collisionLayer;       // Layer for different object types (0=wall, 1=tank, 2=bullet, etc.)

    CollisionBox(sf::FloatRect rect, bool static_obj = true, int layer = 0)
        : bounds(rect), isStatic(static_obj), collisionLayer(layer) {}
};

class CollisionManager {
public:
    CollisionManager(float worldWidth = 1280.f, float worldHeight = 960.f);

    // Check if a rectangle collides with any static or dynamic collider
    bool CheckCollision(const sf::FloatRect& rect, sf::Vector2f& pushback) const;

    // Simple check if two rectangles overlap
    static bool IsColliding(const sf::FloatRect& rect1, const sf::FloatRect& rect2);

    // Calculate the pushback vector to separate two overlapping rectangles
    static sf::Vector2f CalculatePushback(const sf::FloatRect& movingRect, const sf::FloatRect& staticRect);

    // Add a static collision box (walls, fences)
    void AddStaticCollider(const sf::FloatRect& rect, int layer = 0);

    // Clear all dynamic colliders (call at start of each frame)
    void ClearDynamicColliders();

    // Add a dynamic collision box (tanks, moving objects)
    void AddDynamicCollider(const sf::FloatRect& rect, int layer = 1);

    // Create invisible walls at world edges
    void CreateBoundaryWalls(float thickness = 50.f);


private:
    std::vector<CollisionBox> staticColliders;   // Walls, fences, static obstacles
    std::vector<CollisionBox> dynamicColliders;  // Tanks, moving objects

    float worldWidth;
    float worldHeight;
};