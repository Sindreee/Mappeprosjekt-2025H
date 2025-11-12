//
// Created by sindr on 11/12/2025.
//

#ifndef MAPPEOPPGAVE_BEARS_HPP
#define MAPPEOPPGAVE_BEARS_HPP
// C++
// File: include/actors/Bears.hpp
#pragma once

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>
#include <string>

struct MovingBear {
    std::shared_ptr<threepp::Object3D> object;
    float dx = 0.0f;
    float dz = 0.0f;
    float timer = 0.0f;
};

// Loads the bear model once, clones 'count' bears into the scene, and fills 'outBears'.
// Throws std::runtime_error if the model fails to load.
void spawnBears(threepp::Scene& scene,
                std::vector<MovingBear>& outBears,
                int count,
                const std::string& modelPath,
                float scale = 0.1f);

// Simple wandering update (retargets every ~5s).
void updateBears(std::vector<MovingBear>& bears, float dt);


#endif //MAPPEOPPGAVE_BEARS_HPP