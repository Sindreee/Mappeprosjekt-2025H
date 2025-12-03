#include "Bear.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include "Random.hpp"

using namespace threepp;

Bear::Bear(std::shared_ptr<Object3D> mesh) : object_(std::move(mesh)) {
    // Start with a small random movement direction so each bear begins wandering immediate
    auto dest = rnd::uniform2(-0.5f, 0.5f);
    dx_ = dest.first;
    dz_ = dest.second;
}

void Bear::update(float dt) {
    // Accumulate time since last direction change
    timer_ += dt;

    // Every few seconds, pick a new random direction to keep the wander pattern unpredictable.
    if (timer_ >= 5.0f) {
        auto dest = rnd::uniform2(-0.5f, 0.5f);
        dx_ = dest.first;
        dz_ = dest.second;
        timer_ = 0.0f;
    }

    // Move the bear in the current wander direction at a fixed speed.
    const float speed = 100.0f;
    object_->position.x += dx_ * dt * speed;
    object_->position.z += dz_ * dt * speed;

    // Rotate the model so it faces the current movement direction in the XZ-plane.
    if (std::abs(dx_) > 0.01f || std::abs(dz_) > 0.01f) {
        object_->rotation.y = std::atan2(dx_, dz_);
    }
}

std::vector<Bear> createBears(Scene& scene, int count, const std::string& modelPath, float scale) {
    AssimpLoader loader;
    auto model = loader.load(modelPath);

    // Make empty Bear container
    std::vector<Bear> bears;

    // guard againts model failure
    if (!model) return bears;

    // Spawn "count" number of bears, sharing the same base model via cloning
    for (int i = 0; i < count; ++i) {
        auto mesh = (i==0) ? model : model->clone();
        auto pos = rnd::uniform2(-1000.f, 1000.f);

        // Randomize horizontal placement in a large square around the origin.
        mesh->position.set(pos.first, 2, pos.second);
        mesh->scale.set(scale, scale, scale);

        // Enable shadows for all bears
        mesh->traverseType<Mesh>([](Mesh& m){
            m.castShadow = true;
            m.receiveShadow = true;
        });

        scene.add(mesh);
        bears.emplace_back(mesh);
    }

    return bears;
}