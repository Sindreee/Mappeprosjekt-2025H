#include "Wolf.hpp"
#include "GameConstants.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <cmath>
#include "Random.hpp"


using namespace threepp;

Wolf::Wolf(std::shared_ptr<Object3D> mesh, float maxSpeed, float acceleration, Vector3 offset)
    : object_(std::move(mesh)), maxSpeed_(maxSpeed), acceleration_(acceleration), targetOffset_(offset)
{
    // Initialize with a random phase to desynchronize each wolf's procedural movement.
    noiseOffset_ = rnd::uniform(0.0f, 100.0f);
}

void Wolf::takeDamage() {
    if (!alive_) return;
    hits_++;

    // Wolves get faster/more aggressive each time they are hit.
    maxSpeed_ += 20.0f;

    if (hits_ >= hitsToDie_) {
        alive_ = false;
        object_->visible = false;
    }
}

void Wolf::update(float dt, const Vector3& targetPos) {
    if (!alive_) return;

    timeAlive_ += dt;

    // --- Compute a noisy target position around the car ---
    // 1. Start at the car position plus a static per-wolf offset.
    Vector3 baseTarget = targetPos + targetOffset_;

    // 2. Add fast wobble and slower drift to make the path unpredictable.
    float wobbleX = std::sin(timeAlive_ * 2.0f + noiseOffset_) * 20.0f;
    float wobbleZ = std::cos(timeAlive_ * 1.5f + noiseOffset_ * 0.5f) * 20.0f;
    float driftX = std::sin(timeAlive_ * 0.5f) * 50.0f;
    float driftZ = std::cos(timeAlive_ * 0.3f) * 50.0f;

    Vector3 finalTarget = baseTarget;
    finalTarget.x += wobbleX + driftX;
    finalTarget.z += wobbleZ + driftZ;

    // --- Steering behaviour towards the final target ---
    Vector3 direction = finalTarget - object_->position;
    direction.y = 0;

    float dist = direction.length();

    if (dist > 1.0f) { // Don't jitter when very close
        direction.normalize();
        Vector3 desiredVel = direction * maxSpeed_;
        Vector3 steering = desiredVel - velocity_;

        // Limit acceleration for smoother, heavier movement.
        if (steering.length() > acceleration_ * dt) {
            steering.normalize().multiplyScalar(acceleration_ * dt);
        }

        velocity_ += steering;
        velocity_ *= 0.98f; // Friction/damping

        object_->position += velocity_ * dt;

        // Rotate the wolf so it faces its direction of travel.
        if (velocity_.length() > 0.1f) {
            float targetRotation = std::atan2(velocity_.x, velocity_.z);
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

        // Spawn wolves on a ring around the origin, so they converge on the car from all sides.
        float angle = rnd::uniform(0.0f, math::PI * 2.0f);
        float dist = rnd::uniform(300.0f, 600.0f);
        wMesh->position.set(std::cos(angle) * dist, 0.f, std::sin(angle) * dist);

        wMesh->scale.set(5.0f, 5.0f, 5.0f);

        wMesh->traverseType<Mesh>([](Mesh& m){
            m.castShadow = true; m.receiveShadow = true;
        });

        scene.add(wMesh);

        // Give each wolf a unique offset so they don't stack directly on the car.
        Vector3 offset(rnd::uniform(-15.f, 15.f), 0, rnd::uniform(-15.f, 15.f));

        wolves.emplace_back(wMesh, baseSpeed, baseAcc, offset);
    }
    return wolves;
}