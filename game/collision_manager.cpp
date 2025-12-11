//
// Created by Pablo Gonzalez Poblette on 28/10/25.
//

#include "collision_manager.h"
#include <cmath>

CollisionManager::CollisionManager(float worldWidth, float worldHeight)
    : worldWidth(worldWidth), worldHeight(worldHeight) {
    // Create walls on start
    CreateBoundaryWalls();
}

bool CollisionManager::CheckCollision(const sf::FloatRect& rect, sf::Vector2f& pushback) const {
    pushback = {0, 0};
    bool collisionDetected = false;
    float smallestMagnitude = INFINITY;

    // Check against all static colliders
    for (const auto& collider : staticColliders) {
        if (IsColliding(rect, collider.bounds)) {
            sf::Vector2f currentPushback = CalculatePushback(rect, collider.bounds);
            float magnitude = std::sqrt(currentPushback.x * currentPushback.x +
                                      currentPushback.y * currentPushback.y);

            // keep the smallest one so it doesnt over calculate
            if (magnitude < smallestMagnitude) {
                smallestMagnitude = magnitude;
                pushback = currentPushback;
                collisionDetected = true;
            }
        }
    }

    // Check against all dynamic colliders
    for (const auto& collider : dynamicColliders) {
        if (IsColliding(rect, collider.bounds)) {
            sf::Vector2f currentPushback = CalculatePushback(rect, collider.bounds);
            float magnitude = std::sqrt(currentPushback.x * currentPushback.x +
                                      currentPushback.y * currentPushback.y);

            if (magnitude < smallestMagnitude) {
                smallestMagnitude = magnitude;
                pushback = currentPushback;
                collisionDetected = true;
            }
        }
    }

    return collisionDetected;
}

bool CollisionManager::IsColliding(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
    return rect1.position.x < rect2.position.x + rect2.size.x &&
           rect1.position.x + rect1.size.x > rect2.position.x &&
           rect1.position.y < rect2.position.y + rect2.size.y &&
           rect1.position.y + rect1.size.y > rect2.position.y;
}

sf::Vector2f CollisionManager::CalculatePushback(const sf::FloatRect& movingRect,
                                                 const sf::FloatRect& staticRect) {
    sf::Vector2f pushback(0, 0);

    // Calculate overlap on each side
    float overlapLeft = (movingRect.position.x + movingRect.size.x) - staticRect.position.x;
    float overlapRight = (staticRect.position.x + staticRect.size.x) - movingRect.position.x;
    float overlapTop = (movingRect.position.y + movingRect.size.y) - staticRect.position.y;
    float overlapBottom = (staticRect.position.y + staticRect.size.y) - movingRect.position.y;

    // Find the MINIMUN on each axis
    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    float cornerTreshold = 5.0f;
    float overlapDiff = std::abs(minOverlapX - minOverlapY);

    if (overlapDiff < cornerTreshold) {
        // Corner collision - resolve BOTH axes
        if (overlapLeft < overlapRight) {
            pushback.x = -overlapLeft;
        } else {
            pushback.x = overlapRight;
        }

        if (overlapTop < overlapBottom) {
            pushback.y = -overlapTop;
        } else {
            pushback.y = overlapBottom;
        }
    } else {
        // Normal collision
        if (minOverlapX < minOverlapY) {
            //  horizontally
            if (overlapLeft < overlapRight) {
                pushback.x = -overlapLeft;
            } else {
                pushback.x = overlapRight;
            }
        } else {
            //  vertically
            if (overlapTop < overlapBottom) {
                pushback.y = -overlapTop;
            } else {
                pushback.y = overlapBottom;
            }
        }
    }

    return pushback;
}

void CollisionManager::AddStaticCollider(const sf::FloatRect& rect, int layer) {
    staticColliders.emplace_back(rect, true, layer);
}

void CollisionManager::ClearDynamicColliders() {
    dynamicColliders.clear();
}

void CollisionManager::AddDynamicCollider(const sf::FloatRect& rect, int layer) {
    dynamicColliders.emplace_back(rect, false, layer);
}

void CollisionManager::CreateBoundaryWalls(float thickness) {

    // Top wall
    AddStaticCollider(sf::FloatRect(sf::Vector2f(0, -thickness), sf::Vector2f(worldWidth, thickness)));

    // Bottom wall
    AddStaticCollider(sf::FloatRect(sf::Vector2f(0, worldHeight), sf::Vector2f(worldWidth, thickness)));

    // Left wall
    AddStaticCollider(sf::FloatRect(sf::Vector2f(-thickness, 0), sf::Vector2f(thickness, worldHeight)));

    // Right wall
    AddStaticCollider(sf::FloatRect(sf::Vector2f(worldWidth, 0), sf::Vector2f(thickness, worldHeight)));
}