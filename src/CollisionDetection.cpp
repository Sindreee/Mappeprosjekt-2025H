//
// Created by sindr on 11/10/2025.
//

#include "CollisionDetection.hpp"
#include <cmath>

CollisionDetection::CollisionDetection(float collisionRadius)
    : collisionRadius_(collisionRadius) {}

void CollisionDetection::setObstacles(const std::vector<threepp::Vector3>& positions) {
    obstaclePositions_ = positions;
}

void CollisionDetection::addObstacle(const threepp::Vector3& position) {
    obstaclePositions_.push_back(position);
}

bool CollisionDetection::checkCollision(const threepp::Vector3& position) const {
    for (const auto& obstaclePos : obstaclePositions_) {
        float dx = position.x - obstaclePos.x;
        float dz = position.z - obstaclePos.z;
        float distance = std::sqrt(dx * dx + dz * dz);

        if (distance < collisionRadius_) {
            return true;
        }
    }
    return false;
}