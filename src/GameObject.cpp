#include "GameObject.hpp"

// Wrap the given scene object so it can participate in collision and scene management
GameObject::GameObject(std::shared_ptr<threepp::Object3D> mesh)
    : model_(std::move(mesh)) {}

// Register a callback to run when this GameObject is reported as colliding
void GameObject::setOnCollision(CollisionCallback callback) {
    onCollision_ = std::move(callback);
}


// Invoke the collision callback if one is registered
void GameObject::handleCollision() {
    if (onCollision_) {
        onCollision_(*this);
    }
}

// Detach this objects model from the scene graph
void GameObject::removeFromScene(threepp::Scene& scene) {

    if (model_) {
        scene.remove(*model_);
    }
}