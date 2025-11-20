//
// Created by sindr on 11/12/2025.
//

// C++
// File: 'include/bears.hpp'
#include "bears.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <random>
#include <stdexcept>
#include "Random.hpp"

using namespace threepp;

namespace {
    std::pair<float, float> randPos(float minV = -1600.f, float maxV = 1600.f) {
        return rnd::uniform2(minV, maxV);
    }

    std::pair<float, float> rand01() {
        return {rnd::unit(), rnd::unit()};
    }
}

void spawnBears(Scene& scene,
                std::vector<MovingBear>& outBears,
                int count,
                const std::string& modelPath,
                float scale) {
    AssimpLoader loader;
    auto bearModel = loader.load(modelPath);
    if (!bearModel) {
        throw std::runtime_error("Failed to load bear model: " + modelPath);
    }

    outBears.reserve(outBears.size() + static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        auto bearClone = (i == 0) ? bearModel : bearModel->clone();

        auto [x, z] = randPos();
        bearClone->position.set(x, 0, z);
        bearClone->rotation.y = -math::PI * x * z;
        bearClone->scale.set(scale, scale, scale);

        scene.add(bearClone);

        auto [dxr, dzr] = rand01();
        outBears.push_back(MovingBear{
            bearClone,
            dxr - 0.5f,
            dzr - 0.5f,
            0.0f
        });
    }
}

void updateBears(std::vector<MovingBear>& bears, float dt) {
    const float speed = 100.0f;
    for (auto& bear : bears) {
        bear.timer += dt;

        if (bear.timer >= 5.0f) {
            auto [dxr, dzr] = rand01();
            bear.dx = dxr - 0.5f;
            bear.dz = dzr - 0.5f;
            bear.timer = 0.0f;
        }

        bear.object->position.x += bear.dx * dt * speed;
        bear.object->position.z += bear.dz * dt * speed;
    }
}