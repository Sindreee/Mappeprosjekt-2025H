#include "Wolf.hpp"
#include "GameConstants.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <cmath>
#include "Random.hpp" // Make sure Random.hpp is available or use std::rand


using namespace threepp;

Wolf::Wolf(std::shared_ptr<Object3D> mesh, float maxSpeed, float acceleration, Vector3 offset)
    : object_(std::move(mesh)), maxSpeed_(maxSpeed), acceleration_(acceleration), targetOffset_(offset)
{
    // Initialize with a random value to desynchronize wolf movements
    noiseOffset_ = rnd::uniform(0.0f, 100.0f);
}

void Wolf::takeDamage() {
    if (!alive_) return;
    hits_++;
    // Wolves get faster/angrier when hit
    maxSpeed_ += 20.0f;

    if (hits_ >= hitsToDie_) {
        alive_ = false;
        object_->visible = false;
    }
}

void Wolf::update(float dt, const Vector3& targetPos) {
    if (!alive_) return;

    timeAlive_ += dt;

    // --- Unpredictable Movement Logic ---
    // 1. Calculate base target (Car position + static offset)
    Vector3 baseTarget = targetPos + targetOffset_;

    // 2. Add time-varying noise ("Wobble")
    // We use sin/cos at different frequencies to create a chaotic path
    float wobbleX = std::sin(timeAlive_ * 2.0f + noiseOffset_) * 20.0f; // Fast wobble
    float wobbleZ = std::cos(timeAlive_ * 1.5f + noiseOffset_ * 0.5f) * 20.0f; // Slower wobble

    // 3. Add a larger, slower "Patrol" variance
    float driftX = std::sin(timeAlive_ * 0.5f) * 50.0f;
    float driftZ = std::cos(timeAlive_ * 0.3f) * 50.0f;

    Vector3 finalTarget = baseTarget;
    finalTarget.x += wobbleX + driftX;
    finalTarget.z += wobbleZ + driftZ;

    // --- Steering Behavior ---
    Vector3 direction = finalTarget - object_->position;
    direction.y = 0;

    float dist = direction.length();

    if (dist > 1.0f) { // Don't jitter when very close
        direction.normalize();
        Vector3 desiredVel = direction * maxSpeed_;
        Vector3 steering = desiredVel - velocity_;

        // Limit acceleration (makes movement smoother/weightier)
        if (steering.length() > acceleration_ * dt) {
            steering.normalize().multiplyScalar(acceleration_ * dt);
        }

        velocity_ += steering;
        velocity_ *= 0.98f; // Friction/Damping

        object_->position += velocity_ * dt;

        // Rotation: Face direction of movement
        if (velocity_.length() > 0.1f) {
            float targetRotation = std::atan2(velocity_.x, velocity_.z);
            // Smooth rotation could be added here, but direct set is fine for now
            object_->rotation.y = targetRotation;
        }
    }
}

std::vector<Wolf> createWolves(Scene& scene, int count, const std::string& modelPath, float difficultyFactor) {
    AssimpLoader loader;
    auto model = loader.load(modelPath);
    std::vector<Wolf> wolves;

    if (!model) return wolves;

    float baseSpeed = 50.0f + (difficultyFactor - 1) * 40.0f;
    float baseAcc = 150.0f + (difficultyFactor - 1) * 50.0f;

    for (int i = 0; i < count; ++i) {
        auto wMesh = model->clone();

        // Random spawn circle around center
        float angle = rnd::uniform(0.0f, math::PI * 2.0f);
        float dist = rnd::uniform(300.0f, 600.0f);
        wMesh->position.set(std::cos(angle) * dist, 0.f, std::sin(angle) * dist);

        wMesh->scale.set(5.0f, 5.0f, 5.0f);

        wMesh->traverseType<Mesh>([](Mesh& m){
            m.castShadow = true; m.receiveShadow = true;
        });

        scene.add(wMesh);

        // Give each wolf a unique offset so they don't stack directly on the car
        Vector3 offset(rnd::uniform(-15.f, 15.f), 0, rnd::uniform(-15.f, 15.f));

        wolves.emplace_back(wMesh, baseSpeed, baseAcc, offset);
    }
    return wolves;
}