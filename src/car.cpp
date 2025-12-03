#include "car.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <iostream>
#include <cmath>


Car::Car() {
    group_ = std::make_shared<threepp::Group>();
    threepp::AssimpLoader loader;

    // Load the deer model and use it as the visual root of the car.
    deerModel_ = loader.load("Models/Mesh_Deer.obj");
    if (deerModel_) {
        deerModel_->scale.set(0.05f, 0.05f, 0.05f);
        group_->add(deerModel_);
    }

    // Load skateboard as a child of the deer model so it moves together with the deer.
    auto skateboardModel = loader.load("Models/SkateboardBrettOgAkslinger2.obj");
    if (skateboardModel) {
        skateboardModel->scale.set(0.5f, 0.5f, 0.5f);
        skateboardModel->position.set(0, -65.0f, 0);
        if (deerModel_) deerModel_->add(skateboardModel);

        // Predefined local X/Z offsets for the four wheels
        // Given their repetetive behaviour this was easier than placing individually
        std::vector<std::pair<int, std::pair<float, float>>> wheelPos = {
            {0, {70, 70}}, {1, {-70, 70}}, {2, {70, -70}}, {3, {-70, -70}}
        };

        auto wheelMesh = loader.load("Models/TerrainWheel3.obj");
        if (wheelMesh) {
            // Clone the same wheel mesh into each wheel slot and attach to the skateboard.
            for (auto& wp : wheelPos) {
                auto wheel = wheelMesh->clone();
                wheel->position.set(wp.second.first, -8, wp.second.second);
                wheels_[wp.first] = wheel;
                skateboardModel->add(wheel);
            }
        }
    }

    // Enable shadow casting/receiving on all meshes in the hierarchy instead of individually
    group_->traverseType<threepp::Mesh>([](threepp::Mesh& mesh) {
        mesh.castShadow = true;
        mesh.receiveShadow = true;
    });
}

// Dont move car if dead or falling
void Car::setSpeed(float speed) {
    if (isDead()) {
        currentSpeed_ = 0.0f;
        return;
    }
    if (!isFalling_) {
        currentSpeed_ = speed;
    }
}

// Dont rotate car if dead or falling
void Car::setRotation(float rotation) {
    if (!isFalling_ && !isDead()) {
        currentRotation_ = rotation;
    }
}

void Car::applySpeedBoost(float extraSpeed) {
    speedBoost_ = extraSpeed;
}

// Apply a permanent modification to the base speed and clamp to a minimum value
void Car::modifyBaseSpeed(float delta) {
    baseSpeed_ += delta;
    if (baseSpeed_ < 30.0f) baseSpeed_ = 30.0f;
}

float Car::getMaxSpeed() const {
    return baseSpeed_ + speedBoost_;
}


// Damage system for car with terminal ouput mainly for development purposeses and post game information
void Car::takeDamage(int amount) {
    if (hitCooldownTimer_ > 0.0f || isDead()) return;
    health_ -= amount;
    // Bring in CAR_HIT_COOLDOWN from gameConstants for better program tuning control
    hitCooldownTimer_ = Config::CAR_HIT_COOLDOWN;
    std::cout << "Car hit! Health: " << health_ << "/" << Config::CAR_MAX_HEALTH << std::endl;

    if (isDead()) {
        std::cout << "Car died from damage!" << std::endl;
        currentSpeed_ = 0.0f;
    }
}

bool Car::isDead() const { return health_ <= 0; }

void Car::update(float dt) {
    if (hitCooldownTimer_ > 0.0f) hitCooldownTimer_ -= dt;
    updateWheels(dt);
    checkBoundsAndGravity(dt);
}

void Car::updateWheels(float dt) {
    // Skip animation when there is almost no movement or steering.
    if (std::abs(currentSpeed_) < 0.1f && std::abs(currentRotation_) < 0.1f) return;

    // Rotate wheels around X proportional to forward speed and add a bit of differential for steering.
    float speedRotation = currentSpeed_ * dt * 0.1f;
    float turnRotation = currentRotation_ * dt * 0.05f;

    if (wheels_[0]) wheels_[0]->rotation.x -= (speedRotation - turnRotation);
    if (wheels_[2]) wheels_[2]->rotation.x -= (speedRotation - turnRotation);
    if (wheels_[1]) wheels_[1]->rotation.x -= (speedRotation + turnRotation);
    if (wheels_[3]) wheels_[3]->rotation.x -= (speedRotation + turnRotation);
}

// Check if the car is out of bounds and apply gravity if so for falling effect
void Car::checkBoundsAndGravity(float dt) {
    auto pos = group_->position;
    float limit = Config::WORLD_BOUNDS;

    bool outOfBounds = (pos.x < -limit || pos.x > limit || pos.z < -limit || pos.z > limit);

    if (outOfBounds) {
        isFalling_ = true;
        fallVelocityY_ -= Config::GRAVITY * dt;
        group_->position.y += fallVelocityY_ * dt;

        // Once it falls far enough, the car is considered destroyed.
        if (pos.y < -70.0f) {
            health_ = 0;
        }
    } else {
        // Inside bounds the car is pinned to the ground plane.
        isFalling_ = false;
        fallVelocityY_ = 0.0f;
        group_->position.y = 0.0f;
    }
}


// Reset all car parameters to initial state when restarting the game in game.cpp
void Car::reset() {
    group_->position.copy(spawnPos_);
    group_->rotation.set(0,0,0);
    health_ = Config::CAR_MAX_HEALTH;
    baseSpeed_ = Config::CAR_BASE_SPEED;
    speedBoost_ = 0.0f;
    currentSpeed_ = 0.0f;
    currentRotation_ = 0.0f;
    fallVelocityY_ = 0.0f;
    isFalling_ = false;
    hitCooldownTimer_ = 0.0f;
}