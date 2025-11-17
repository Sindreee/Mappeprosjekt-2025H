//
// Created by sindr on 11/17/2025.
//

#include "bullet.hpp"
#include <threepp/loaders/AssimpLoader.hpp>

using namespace threepp;

Bullet::Bullet(const std::string& modelPath, const Vector3& startPos,
               const Vector3& direction, const float rotation, const float speed, const float lifetime)
    : lifetime_(lifetime) {

    AssimpLoader loader;
    model_ = loader.load(modelPath);

    if (model_) {
        model_->position.copy(startPos);
        model_->rotation.y = rotation + math::PI; // Align bullet with turret
        model_->scale.set(0.2f, 0.2f, 0.2f);
        velocity_ = direction;
        velocity_.normalize();
        velocity_.multiplyScalar(speed);
    }
}

void Bullet::update(float dt) {
    if (!model_) return;

    elapsed_ += dt;

    model_->position.x += velocity_.x * dt;
    model_->position.y += velocity_.y * dt;
    model_->position.z += velocity_.z * dt;
}

bool Bullet::isAlive() const {
    return elapsed_ < lifetime_ && model_ != nullptr;
}

std::shared_ptr<Object3D> Bullet::getModel() const {
    return model_;
}

Vector3 Bullet::getPosition() const {
    return model_ ? model_->position : Vector3();
}