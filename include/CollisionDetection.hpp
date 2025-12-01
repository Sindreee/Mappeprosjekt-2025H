#ifndef COLLISIONDETECTION_HPP
#define COLLISIONDETECTION_HPP

#include <threepp/threepp.hpp>
#include <vector>
#include <functional>
#include <memory>

enum class ColliderType {
    CAR,
    TREE,
    MUSHROOM,
    BEAR,
    WOLF,
    BULLET
};

struct Collider {
    std::shared_ptr<threepp::Object3D> object;
    float radius = 0.0f;
    ColliderType type;
    bool active = true;
    std::function<void(const Collider&, const Collider&)> onCollision;
};

class CollisionDetection {
public:
    explicit CollisionDetection(float collisionRadius = 7.0f);

    int addCollider(std::shared_ptr<threepp::Object3D> object,
                    float radius,
                    ColliderType type,
                    std::function<void(const Collider&, const Collider&)> onCollision = {});

    // Endring her: ignoreObject pointer
    bool isBlocked(const threepp::Vector3& position, float radius, const threepp::Object3D* ignoreObject = nullptr) const;

    void processAllPairs();
    void removeCollidersOfType(ColliderType type);
    void cleanupInactive();

private:
    void processTypePair(ColliderType a, ColliderType b);

    float collisionRadius_;
    std::vector<Collider> colliders_;
};

#endif