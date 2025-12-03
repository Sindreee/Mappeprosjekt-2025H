#include "carKeyController.hpp"
#include <cmath>

using namespace threepp;

// The Keycontrollers key input registration is mainly based on the threepp Youbot code example
KeyController::KeyController(Object3D& object, CollisionDetection& collisionDetection,
                             float acceleration, float maxSpeed, float friction, float turnSpeed)
    : object_(&object), collisionDetection_(&collisionDetection),
      acceleration_(acceleration), maxSpeed_(maxSpeed),
      friction_(friction), turnSpeed_(turnSpeed) {}

// tracking key presses for WASD
void KeyController::onKeyPressed(KeyEvent evt) {
    if (evt.key == Key::W) keyState_.up = true;
    else if (evt.key == Key::S) keyState_.down = true;
    else if (evt.key == Key::D) keyState_.right = true;
    else if (evt.key == Key::A) keyState_.left = true;
}

// Tracking key releases for WASD
void KeyController::onKeyReleased(KeyEvent evt) {
    if (evt.key == Key::W) keyState_.up = false;
    else if (evt.key == Key::S) keyState_.down = false;
    else if (evt.key == Key::D) keyState_.right = false;
    else if (evt.key == Key::A) keyState_.left = false;
}

void KeyController::update(float dt) {

    // Changes the cars velocity based on key inputs
    if (keyState_.up) velocity_ -= acceleration_ * dt;
    if (keyState_.down) velocity_ += acceleration_ * dt;

    // Clamp velocity to current speed limits.
    if (velocity_ > maxSpeed_) velocity_ = maxSpeed_;
    if (velocity_ < -maxSpeed_) velocity_ = -maxSpeed_;

    // Apply simple friction when no acceleration keys are held
    if (!keyState_.up && !keyState_.down) {
        velocity_ *= friction_;
        if (std::abs(velocity_) < 0.1f) velocity_ = 0.0f;
    }

    // Apply yaw rotation from left/right input.
    if (keyState_.right) object_->rotation.y -= turnSpeed_ * dt;
    if (keyState_.left) object_->rotation.y += turnSpeed_ * dt;

    // Calculete the direction vector based on the cars rotation
    float dirX = -std::sin(object_->rotation.y);
    float dirZ = -std::cos(object_->rotation.y);

    // Move the object along its facing direction in the XZ-plane.
    Vector3 newPos = object_->position;
    newPos.x += dirX * velocity_ * dt;
    newPos.z += dirZ * velocity_ * dt;

    // If the car is not colliding with anything, update its position
    // Otherwise, bounce back slightly at half the speed
    if (!collisionDetection_->isBlocked(newPos, 5.0f, object_)) {
        object_->position = newPos;
    } else {
        velocity_ = -velocity_ * 0.5f;
    }
}

// Returns the current speed of the car
float KeyController::getSpeed() const { return velocity_; }

// Function for getting the cars rotation
float KeyController::getRotation() const {
    float rotation = 0.0f;
    if (keyState_.left) rotation += 1.0f;
    if (keyState_.right) rotation -= 1.0f;
    return rotation;
}

// Sets the maximum speed of the car (changes alot during gameplay)
void KeyController::setMaxSpeed(float maxSpeed) {
    maxSpeed_ = maxSpeed;
    if (velocity_ > maxSpeed_) velocity_ = maxSpeed_;
    if (velocity_ < -maxSpeed_) velocity_ = -maxSpeed_;
}

// Adjusts the maximum speed of the car by a delta value
void KeyController::adjustMaxSpeed(float delta) {
    setMaxSpeed(maxSpeed_ + delta);
}


// Returns the current maximum speed of the car
float KeyController::getMaxSpeed() const { return maxSpeed_; }