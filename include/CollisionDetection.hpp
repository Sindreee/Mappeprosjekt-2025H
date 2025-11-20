//
// Created by sindr on 11/10/2025.
//

#ifndef MAPPEOPPGAVE_COLLISIONDETECTION_HPP
#define MAPPEOPPGAVE_COLLISIONDETECTION_HPP

#include <threepp/threepp.hpp>
#include <vector>
#include <functional>
#include <memory>

// New: generalized collider types
enum class ColliderType {
    CAR,
    TREE,
    MUSHROOM,
    BEAR,
    WOLF,
    BULLET
};

struct Collider {
    std::shared_ptr<threepp::Object3D> object; // tracked transform
    float radius = 0.0f;
    ColliderType type{ColliderType::TREE};
    bool active = true;
    // Optional per-collision callback. Called with (self, other)
    std::function<void(const Collider&, const Collider&)> onCollision;
};

class CollisionDetection {
public:
    CollisionDetection(float collisionRadius = 7.0f);

    // Existing tree obstacle API (used by movement controller)
    void setObstacles(const std::vector<threepp::Vector3>& positions);
    void addObstacle(const threepp::Vector3& position);
    bool checkCollision(const threepp::Vector3& position) const; // movement blocking vs obstacles

    // New generic collider registration
    int addCollider(std::shared_ptr<threepp::Object3D> object,
                    float radius,
                    ColliderType type,
                    std::function<void(const Collider&, const Collider&)> onCollision = {});

    void deactivateCollider(int id); // mark inactive without erasing (safe during iteration)

    // Process collisions between two sets of types (e.g. CAR vs BEAR, BULLET vs WOLF)
    void processTypePair(ColliderType a, ColliderType b);

    // Convenience to process several common pairs in one call
    void processAllPairs();

    // Access (optional future use)
    const std::vector<Collider>& colliders() const { return colliders_; }

private:
    std::vector<threepp::Vector3> obstaclePositions_; // static obstacles for movement
    float collisionRadius_;

    std::vector<Collider> colliders_; // dynamic / generic colliders
};

#endif //MAPPEOPPGAVE_COLLISIONDETECTION_HPP