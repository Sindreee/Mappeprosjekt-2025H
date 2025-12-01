#ifndef CAR_HPP
#define CAR_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <array>
#include "GameConstants.hpp"

class Car {
public:
    Car();

    // Input og bevegelse
    void setSpeed(float speed);
    void setRotation(float rotation);

    // Hovedoppdatering
    void update(float dt);

    // Gameplay logikk
    void takeDamage(int amount);
    bool isDead() const;
    int getHealth() const { return health_; }
    bool isFalling() const { return isFalling_; }

    void reset();

    // Speed system
    void applySpeedBoost(float extraSpeed); // For mushrooms
    void modifyBaseSpeed(float delta);      // For bears (penalty)
    float getMaxSpeed() const;

    // Getters
    std::shared_ptr<threepp::Group> getGroup() { return group_; }
    threepp::Vector3 getPosition() const { return group_->position; }

private:
    void updateWheels(float dt);
    void checkBoundsAndGravity(float dt);

    std::shared_ptr<threepp::Group> group_;
    std::shared_ptr<threepp::Object3D> deerModel_;
    std::shared_ptr<threepp::Object3D> skateboardModel_;
    std::array<std::shared_ptr<threepp::Object3D>, 4> wheels_;

    // Tilstand
    float currentSpeed_ = 0.0f;
    float currentRotation_ = 0.0f;
    threepp::Vector3 spawnPos_{0,0,0};

    // Gameplay stats
    int health_ = Config::CAR_MAX_HEALTH;
    float hitCooldownTimer_ = 0.0f;

    // Fysikk / Fart
    float fallVelocityY_ = 0.0f;
    bool isFalling_ = false;

    float baseSpeed_ = Config::CAR_BASE_SPEED; // Mutable base speed
    float speedBoost_ = 0.0f;                  // Temporary boost
};

#endif // CAR_HPP