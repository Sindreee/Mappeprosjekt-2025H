#ifndef COLLISIONDETECTION_HPP
#define COLLISIONDETECTION_HPP

#include <threepp/threepp.hpp>
#include <vector>
#include <functional>
#include <memory>

// Types of colliders used by the simple 2D (XZ-plane) collision system
enum class ColliderType {
    CAR,
    TREE,
    MUSHROOM,
    BEAR,
    WOLF,
    BULLET
};

// Lightweight handle describing a collidable object in the world
struct Collider {
    std::shared_ptr<threepp::Object3D> object;       // Scene object to test against
    float radius = 0.0f;                             // Collision radius in world units
    ColliderType type;                               // Logical type used for filtering
    bool active = true;                              // Inactive colliders are skipped
    std::function<void(const Collider&, const Collider&)> onCollision; // Optional hit callback
};

// Simple XZ-plane sphere-based collision manager with callback-enabled colliders
// the game selects which collider-type pairs to test in processAllPairs().
class CollisionDetection {
public:
    CollisionDetection() = default;

    // Register a new collider around an Object3D and return its index
    int addCollider(std::shared_ptr<threepp::Object3D> object,
                    float radius,
                    ColliderType type,
                    std::function<void(const Collider&, const Collider&)> onCollision = {});

    // Test whether a sphere at the given position is blocked by any solid collider.
    // "ignoreObject" is used to skip the moving object itself when doing the query.
    bool isBlocked(const threepp::Vector3& position, float radius, const threepp::Object3D* ignoreObject = nullptr) const;

    // Check all registered collider pairs that are relevant for gameplay and fire callbacks.
    void processAllPairs();

    // Mark all colliders of a given type as inactive.
    void removeCollidersOfType(ColliderType type);

    // Remove all inactive colliders from the internal list.
    void cleanupInactive();

private:
    void processTypePair(ColliderType a, ColliderType b);
    std::vector<Collider> colliders_;
};

#endif