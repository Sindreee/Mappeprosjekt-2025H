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

int CollisionDetection::addCollider(std::shared_ptr<threepp::Object3D> object,
                                    float radius,
                                    ColliderType type,
                                    std::function<void(const Collider&, const Collider&)> onCollision) {
    colliders_.push_back(Collider{object, radius, type, true, onCollision});
    return static_cast<int>(colliders_.size() - 1);
}

void CollisionDetection::deactivateCollider(int id) {
    if (id >= 0 && id < static_cast<int>(colliders_.size())) {
        colliders_[id].active = false;
    }
}

void CollisionDetection::removeCollidersOfType(ColliderType type) {
    for (auto& c : colliders_) {
        if (c.type == type) c.active = false;
    }
}

void CollisionDetection::cleanupInactive() {
    colliders_.erase(
        std::remove_if(colliders_.begin(), colliders_.end(), [](const Collider& c){ return !c.active; }),
        colliders_.end());
}

void CollisionDetection::processTypePair(ColliderType a, ColliderType b) {
    // Collect indices by type for quick iteration
    std::vector<int> aIdx; aIdx.reserve(colliders_.size());
    std::vector<int> bIdx; bIdx.reserve(colliders_.size());
    for (int i = 0; i < static_cast<int>(colliders_.size()); ++i) {
        if (!colliders_[i].active || !colliders_[i].object) continue;
        if (!colliders_[i].object->visible) continue; // ignore hidden objects
        if (colliders_[i].type == a) aIdx.push_back(i);
        else if (colliders_[i].type == b) bIdx.push_back(i);
    }

    for (int ia : aIdx) {
        for (int ib : bIdx) {
            // Avoid processing the same collider against itself
            if (ia == ib) continue;
            const auto& ca = colliders_[ia];
            const auto& cb = colliders_[ib];
            const auto& pa = ca.object->position;
            const auto& pb = cb.object->position;
            float dx = pa.x - pb.x;
            float dz = pa.z - pb.z;
            float dist2 = dx * dx + dz * dz;
            float radiusSum = ca.radius + cb.radius;
            if (dist2 <= radiusSum * radiusSum) {
                if (ca.onCollision) ca.onCollision(ca, cb);
                if (cb.onCollision) cb.onCollision(cb, ca);
            }
        }
    }
}

void CollisionDetection::processAllPairs() {
    // Common pairs we care about for current gameplay
    processTypePair(ColliderType::CAR, ColliderType::BEAR);
    processTypePair(ColliderType::CAR, ColliderType::WOLF);
    processTypePair(ColliderType::CAR, ColliderType::MUSHROOM);
    processTypePair(ColliderType::BULLET, ColliderType::WOLF);
}
