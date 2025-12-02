//
// Wolf.hpp
//
// This file defines the Wolf class, which represents a fast melee enemy in the game.
// Wolves chase the player (or car) using a noisy steering behavior, making them dynamic
// and unpredictable opponents. Each wolf has individual speed, acceleration, and a random
// offset to prevent them from overlapping in their movements.
//
// The Wolf class includes methods for updating the wolf's position, applying damage,
// and retrieving the wolf's current state (alive, number of hits, model object).
//
// Additionally, there is a createWolves function declared, which is used to load a wolf
// model and spawn a specified number of wolves with attributes derived from a given
// difficulty factor.
//
// Created by sindr on 11/18/2025.
//

#ifndef MAPPEOPPGAVE_WOLF_HPP
#define MAPPEOPPGAVE_WOLF_HPP
#include <threepp/threepp.hpp>
#include <memory>
#include <string>
#include <vector>

namespace threepp { class Scene; }

// Fast melee enemy that chases the car using a noisy steering behaviour.
// Each wolf has its own speed, acceleration, and random offset so they don't overlap.
class Wolf {
public:
    Wolf(std::shared_ptr<threepp::Object3D> mesh,
         float maxSpeed,
         float acceleration,
         threepp::Vector3 offset);

    // Steer towards the target position plus offset, with added noise and drift.
    void update(float dt, const threepp::Vector3& targetPos);

    // Apply one hit of damage; wolves speed up when hurt and die after enough hits.
    void takeDamage();

    bool isAlive() const { return alive_; }
    int getHits() const { return hits_; }
    std::shared_ptr<threepp::Object3D> getModel() const { return object_; }

    void setMaxSpeed(float speed) { maxSpeed_ = speed; }

private:
    std::shared_ptr<threepp::Object3D> object_;
    threepp::Vector3 velocity_{0, 0, 0};
    float maxSpeed_;
    float acceleration_;
    threepp::Vector3 targetOffset_;

    float noiseOffset_;      // Per-wolf phase offset for the noise functions
    float timeAlive_ = 0.0f; // Accumulated time for procedural movement

    int hits_ = 0;
    bool alive_ = true;
    const int hitsToDie_ = 5;
};

// Load a wolf model and spawn a number of wolves with stats derived from the difficulty factor.
std::vector<Wolf> createWolves(threepp::Scene& scene, int count, const std::string& modelPath, float difficultyFactor);

#endif //MAPPEOPPGAVE_WOLF_HPP