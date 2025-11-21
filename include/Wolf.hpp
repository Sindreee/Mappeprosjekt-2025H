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
    threepp::Vector3 velocity{0, 0, 0};
    float maxSpeed;
    float acceleration;
    threepp::Vector3 targetOffset{0, 0, 0};
    int hits = 0;            // bullet collision hits
    bool alive = true;       // becomes false after 5 bullet hits
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