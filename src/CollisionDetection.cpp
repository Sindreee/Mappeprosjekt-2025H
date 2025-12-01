#include "CollisionDetection.hpp"
#include <cmath>
#include <algorithm>

CollisionDetection::CollisionDetection(float collisionRadius)
    : collisionRadius_(collisionRadius) {}

int CollisionDetection::addCollider(std::shared_ptr<threepp::Object3D> object,
                                    float radius,
                                    ColliderType type,
                                    std::function<void(const Collider&, const Collider&)> onCollision) {
    colliders_.push_back(Collider{object, radius, type, true, onCollision});
    return static_cast<int>(colliders_.size() - 1);
}

// Endring her: Implementer ignore-logikken
bool CollisionDetection::isBlocked(const threepp::Vector3& position, float checkRadius, const threepp::Object3D* ignoreObject) const {
    for (const auto& col : colliders_) {
        // Hvis collideren ikke er aktiv, er sopp/kule, ELLER er objektet selv -> Hopp over
        if (!col.active ||
            col.type == ColliderType::MUSHROOM ||
            col.type == ColliderType::BULLET ||
            col.object.get() == ignoreObject) { // <-- VIKTIG LINJE
            continue;
        }
        
        float dx = position.x - col.object->position.x;
        float dz = position.z - col.object->position.z;
        float distSq = dx*dx + dz*dz;
        float radiusSum = checkRadius + col.radius;
        
        if (distSq < radiusSum * radiusSum) {
            return true;
        }
    }
    return false;
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
    for (size_t i = 0; i < colliders_.size(); ++i) {
        if (!colliders_[i].active || colliders_[i].type != a) continue;
        
        for (size_t j = 0; j < colliders_.size(); ++j) {
            if (i == j || !colliders_[j].active || colliders_[j].type != b) continue;
            
            const auto& ca = colliders_[i];
            const auto& cb = colliders_[j];
            
            float dx = ca.object->position.x - cb.object->position.x;
            float dz = ca.object->position.z - cb.object->position.z;
            float distSq = dx*dx + dz*dz;
            float radiusSum = ca.radius + cb.radius;
            
            if (distSq <= radiusSum * radiusSum) {
                if (ca.onCollision) ca.onCollision(ca, cb);
                if (cb.onCollision) cb.onCollision(cb, ca);
            }
        }
    }
}

void CollisionDetection::processAllPairs() {
    processTypePair(ColliderType::CAR, ColliderType::BEAR);
    processTypePair(ColliderType::CAR, ColliderType::WOLF);
    processTypePair(ColliderType::CAR, ColliderType::MUSHROOM);
    processTypePair(ColliderType::BULLET, ColliderType::WOLF);
}