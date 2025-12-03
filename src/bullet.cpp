#include "bullet.hpp"
#include <threepp/loaders/AssimpLoader.hpp>


using namespace threepp;

// Construct a bullet at the given start position, moving in the specified direction
Bullet::Bullet(const std::string& modelPath, const Vector3& startPos,
               const Vector3& direction, const float rotation, const float speed, const float lifetime)
    : lifetime_(lifetime) {

    //Load the modelPath with assimp and store in model_
    AssimpLoader loader;
    model_ = loader.load(modelPath);


    // Only run if model loaded sucessfully
    if (model_) {

        // Place the bullet on startPos (inside the turret) and size it properly
        model_->scale.set(0.4f, 0.4f, 0.4f);
        model_->position.copy(startPos);

        //rotate the bulet to face the same direction as the turret
        model_->rotation.y = rotation + math::PI;

        // Compute a normalized velocity vector in world space and scale by speed.
        velocity_ = direction;
        velocity_.normalize();
        velocity_.multiplyScalar(speed);
    }
}


void Bullet::update(float dt) {
    if (!model_) return; // Nothing to update if the mesh failed to load.

    // Acumulate lifetime to know when the bullet should expire.
    elapsed_ += dt;

    // Integrate position in the XZ-plane with a constant velocity
    model_->position.x += velocity_.x * dt;
    model_->position.z += velocity_.z * dt;
}

// Helper to check if the bullet is still alive
bool Bullet::isAlive() const {
    return elapsed_ < lifetime_ && model_ != nullptr;
}

// Access the underlying Threepp object for bullet
std::shared_ptr<Object3D> Bullet::getModel() const {
    return model_;
}

// Get the current position of the bullet
Vector3 Bullet::getPosition() const {
    return model_ ? model_->position : Vector3();
}