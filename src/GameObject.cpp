//
// Created by sindr on 11/17/2025.
//
#include "GameObject.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <cmath>

GameObject::GameObject(const std::string& modelPath,
                       const threepp::Vector3& position,
                       const threepp::Vector3& scale,
                       float collisionRadius,
                       GameObjectType type)
    : collisionRadius_(collisionRadius), type_(type) {


    threepp::AssimpLoader loader;
    model_ = loader.load(modelPath);

    if (model_) {
        model_->position.copy(position);
        model_->scale.copy(scale);
    }
}

void GameObject::setOnCollision(CollisionCallback callback) {
    onCollision_ = callback;
}
// Add this new constructor here:
GameObject::GameObject(std::shared_ptr<threepp::Object3D> mesh,
                       float collisionRadius,
                       GameObjectType type)
    : model_(mesh), collisionRadius_(collisionRadius), type_(type), active_(true) {}


bool GameObject::checkCollision(const threepp::Vector3& position) const {
    if (!active_ || !model_) return false;

    float dx = position.x - model_->position.x;
    float dz = position.z - model_->position.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    return distance < collisionRadius_;
}

void GameObject::handleCollision() {
    if (onCollision_ && active_) {
        onCollision_(*this);
    }
}

void GameObject::removeFromScene(threepp::Scene& scene) {
    if (model_) {
        scene.remove(*model_);
        active_ = false;
    }
}