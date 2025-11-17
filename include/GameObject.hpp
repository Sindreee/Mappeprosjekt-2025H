//
// Created by sindr on 11/17/2025.
//

#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <threepp/threepp.hpp>
#include <functional>
#include <memory>
#include <string>

enum class GameObjectType {
    TREE,      // Blocks movement
    MUSHROOM,  // Causes blur effect
    NEUTRAL    // No special behavior
};

class GameObject {
public:
    using CollisionCallback = std::function<void(GameObject&)>;

    GameObject(const std::string& modelPath,
               const threepp::Vector3& position,
               const threepp::Vector3& scale,
               float collisionRadius,
               GameObjectType type = GameObjectType::NEUTRAL);

    GameObject(std::shared_ptr<threepp::Object3D> mesh,
           float collisionRadius,
           GameObjectType type);

    void setOnCollision(CollisionCallback callback);
    bool checkCollision(const threepp::Vector3& position) const;
    void handleCollision();
    void removeFromScene(threepp::Scene& scene);

    std::shared_ptr<threepp::Object3D> getModel() const { return model_; }
    const threepp::Vector3& getPosition() const { return model_->position; }
    float getCollisionRadius() const { return collisionRadius_; }
    bool isActive() const { return active_; }
    void setActive(bool active) { active_ = active; }
    GameObjectType getType() const { return type_; }

private:
    std::shared_ptr<threepp::Object3D> model_;
    float collisionRadius_;
    bool active_ = true;
    GameObjectType type_;
    CollisionCallback onCollision_;
};

#endif // GAMEOBJECT_HPP
