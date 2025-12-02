#include "GameObject.hpp"

GameObject::GameObject(std::shared_ptr<threepp::Object3D> mesh)
    : model_(std::move(mesh)) {}

void GameObject::setOnCollision(CollisionCallback callback) {
    onCollision_ = std::move(callback);
}

void GameObject::handleCollision() {
    // Forward the collision event to the user-provided callback, if present.
    if (onCollision_) {
        onCollision_(*this);
    }
}

void GameObject::removeFromScene(threepp::Scene& scene) {
    // Detach this object's model from the scene graph.
    if (model_) {
        scene.remove(*model_);
    }
}