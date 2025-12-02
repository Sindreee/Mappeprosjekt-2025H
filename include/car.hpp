#ifndef CAR_HPP
#define CAR_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <array>
#include "GameConstants.hpp"

// Represents the player-controlled vehicle (a deer on a skateboard).
// Owns visual models, health, speed parameters and simple gravity/out-of-bounds logic.
class Car {
public:
    Car();

    // Set the current forward speed as computed by the input controller.
    void setSpeed(float speed);

    // Set the current steering input, used mainly to animate wheels.
    void setRotation(float rotation);

    // Per-frame update for wheel animation, gravity and bounds checking.
    void update(float dt);

    // Apply damage to the car, respecting a short invulnerability cooldown.
    void takeDamage(int amount);

    // Returns true when the car has no health left.
    bool isDead() const;

    int getHealth() const { return health_; }
    bool isFalling() const { return isFalling_; }

    // Reset the car back to its spawn state (position, speed, health).
    void reset();

    // Temporarily increase the car's maximum speed (e.g. from mushrooms).
    void applySpeedBoost(float extraSpeed);

    // Permanently adjust the car's base maximum speed (e.g. from bear hits).
    void modifyBaseSpeed(float delta);

    // Current maximum speed (base speed plus any temporary boost).
    float getMaxSpeed() const;

    // Root group of the car hierarchy, used to attach to the scene and camera.
    std::shared_ptr<threepp::Group> getGroup() { return group_; }

private:
    void updateWheels(float dt);
    void checkBoundsAndGravity(float dt);

    std::shared_ptr<threepp::Group> group_;
    std::shared_ptr<threepp::Object3D> deerModel_;
    std::array<std::shared_ptr<threepp::Object3D>, 4> wheels_;

    float currentSpeed_ = 0.0f;      // Forward/backward input from the controller
    float currentRotation_ = 0.0f;   // Steering input from the controller
    threepp::Vector3 spawnPos_{0,0,0};

    int health_ = Config::CAR_MAX_HEALTH;
    float hitCooldownTimer_ = 0.0f;

    float fallVelocityY_ = 0.0f;     // Vertical velocity when falling out of bounds
    bool isFalling_ = false;

    float baseSpeed_ = Config::CAR_BASE_SPEED; // Base maximum speed without boosts
    float speedBoost_ = 0.0f;                  // Extra speed added by collectibles
};

#endif // CAR_HPP