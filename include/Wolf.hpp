//
// Created by sindr on 11/18/2025.
//

#ifndef MAPPEOPPGAVE_WOLF_HPP
#define MAPPEOPPGAVE_WOLF_HPP

#include <threepp/threepp.hpp>
#include <vector>
#include <memory>

struct Wolf {
    std::shared_ptr<threepp::Object3D> object;
    threepp::Vector3 velocity{0, 0, 0};  // Current velocity for acceleration
    float maxSpeed;
    float acceleration;

    // Variation parameters
    threepp::Vector3 targetOffset{0, 0, 0};   // Constant offset added to target position per wolf
                          // Internal time accumulator for wobble
};

void spawnWolves(threepp::Scene& scene,
                 std::vector<Wolf>& outWolves,
                 int count,
                 const std::string& modelPath,
                 float scale,
                 float maxSpeed,
                 float acceleration);

void updateWolves(std::vector<Wolf>& wolves,
                  float dt,
                  const threepp::Vector3& targetPos);

#endif //MAPPEOPPGAVE_WOLF_HPP