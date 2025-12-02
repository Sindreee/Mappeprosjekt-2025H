#include "carKeyController.hpp"
#include <cmath>

using namespace threepp;

KeyController::KeyController(Object3D& object, CollisionDetection& collisionDetection,
                             float acceleration, float maxSpeed, float friction, float turnSpeed)
    : object_(&object), collisionDetection_(&collisionDetection),
      acceleration_(acceleration), maxSpeed_(maxSpeed),
      friction_(friction), turnSpeed_(turnSpeed) {}

void KeyController::onKeyPressed(KeyEvent evt) {
    if (evt.key == Key::W) keyState_.up = true;
    else if (evt.key == Key::S) keyState_.down = true;
    else if (evt.key == Key::D) keyState_.right = true;
    else if (evt.key == Key::A) keyState_.left = true;
}

void KeyController::onKeyReleased(KeyEvent evt) {
    if (evt.key == Key::W) keyState_.up = false;
    else if (evt.key == Key::S) keyState_.down = false;
    else if (evt.key == Key::D) keyState_.right = false;
    else if (evt.key == Key::A) keyState_.left = false;
}

void KeyController::update(float dt) {
    // Integrate forward/backward input into a 1D velocity along the car's local forward axis.
    if (keyState_.up) velocity_ -= acceleration_ * dt;
    if (keyState_.down) velocity_ += acceleration_ * dt;

    // Clamp velocity to current speed limits.
    if (velocity_ > maxSpeed_) velocity_ = maxSpeed_;
    if (velocity_ < -maxSpeed_) velocity_ = -maxSpeed_;

    // Apply simple friction when no acceleration keys are held.
    if (!keyState_.up && !keyState_.down) {
        velocity_ *= friction_;
        if (std::abs(velocity_) < 0.1f) velocity_ = 0.0f;
    }

    // Apply yaw rotation from left/right input.
    if (keyState_.right) object_->rotation.y -= turnSpeed_ * dt;
    if (keyState_.left) object_->rotation.y += turnSpeed_ * dt;

    // Move the object along its facing direction in the XZ-plane.
    float dirX = -std::sin(object_->rotation.y);
    float dirZ = -std::cos(object_->rotation.y);

    Vector3 newPos = object_->position;
    newPos.x += dirX * velocity_ * dt;
    newPos.z += dirZ * velocity_ * dt;

    // Query the collision system, ignoring the car itself, to see if the new position is blocked.
    if (!collisionDetection_->isBlocked(newPos, 5.0f, object_)) {
        object_->position = newPos;
    } else {
        // On collision, bounce back slightly and lose some speed.
        velocity_ = -velocity_ * 0.5f;
    }
}

float KeyController::getSpeed() const { return velocity_; }

float KeyController::getRotation() const {
    float rotation = 0.0f;
    if (keyState_.left) rotation += 1.0f;
    if (keyState_.right) rotation -= 1.0f;
    return rotation;
}

void KeyController::setMaxSpeed(float maxSpeed) {
    maxSpeed_ = maxSpeed;
    if (velocity_ > maxSpeed_) velocity_ = maxSpeed_;
    if (velocity_ < -maxSpeed_) velocity_ = -maxSpeed_;
}

void KeyController::adjustMaxSpeed(float delta) {
    setMaxSpeed(maxSpeed_ + delta);
}

float KeyController::getMaxSpeed() const { return maxSpeed_; }