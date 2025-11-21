//
// Created by sindr on 11/18/2025.
//
#include "Wolf.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <random>

using namespace threepp;

void spawnWolves(Scene& scene, std::vector<Wolf>& outWolves, int count,
                 const std::string& modelPath, float scale, float maxSpeed, float acceleration) {
    AssimpLoader loader;
    auto wolfModel = loader.load(modelPath);

    if (!wolfModel) {
        return; // Failed to load
    }

    outWolves.reserve(count);

    // Spawn positions for 3 wolves
    Vector3 spawnPositions[] = {
        {300.0f, 0.0f, 300.0f},
        {350.0f, 0.0f, 320.0f},
        {230.0f, 0.0f, 330.0f}
    };

    // Predefined offsets so they don't stack directly on target
    Vector3 targetOffsets[] = {
        { 6.0f, 0.0f,  10.0f},
        {0.0f, 0.0f, -15.0f},
        { -5.0f, 0.0f, -7.0f}
    };



    for (int i = 0; i < count && i < 3; ++i) {
        auto wolf = (i == 0) ? wolfModel : wolfModel->clone();
        wolf->position.copy(spawnPositions[i]);
        wolf->scale.set(scale, scale, scale);
        scene.add(wolf);

        Wolf w{wolf, Vector3{0, 0, 0}, maxSpeed, acceleration};
        w.targetOffset = targetOffsets[i];


        outWolves.push_back(std::move(w));
    }
}

void updateWolves(std::vector<Wolf>& wolves, float dt, const threepp::Vector3& targetPos) {
    for (auto& wolf : wolves) {
        if (!wolf.alive) continue; // dead wolves do not move


        // Compute wobble vector in the local XZ plane relative to world axes


        // Adjusted target includes constant offset and wobble
        Vector3 adjustedTarget = targetPos + wolf.targetOffset ;

        // Calculate direction to adjusted target
        Vector3 direction = adjustedTarget - wolf.object->position;
        direction.y = 0;  // Keep on ground plane

        float distanceToTarget = direction.length();

        if (distanceToTarget > 0.1f) {
            direction.normalize();

            // Apply acceleration towards target
            Vector3 desiredVelocity = direction * wolf.maxSpeed;
            Vector3 steering = desiredVelocity - wolf.velocity;

            // Limit acceleration
            float steeringLength = steering.length();
            float maxDeltaV = wolf.acceleration * dt;
            if (steeringLength > maxDeltaV) {
                steering.normalize();
                steering *= maxDeltaV;
            }

            wolf.velocity += steering;

            // Apply velocity with damping for smooth movement
            wolf.velocity *= 0.985f; // Slight damping

            // Update position
            wolf.object->position += wolf.velocity * dt;

            // Make wolf face movement direction
            if (wolf.velocity.length() > 0.1f) {
                float angle = std::atan2(wolf.velocity.x, wolf.velocity.z);
                wolf.object->rotation.y = angle;
            }
        } else {
            // Slow down when very close
            wolf.velocity *= 0.9f;
        }
    }
}