#include "Bear.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include "Random.hpp"

using namespace threepp;

Bear::Bear(std::shared_ptr<Object3D> mesh) : object_(std::move(mesh)) {
    // Start with a random small movement direction so bears don't stand still.
    auto dest = rnd::uniform2(-0.5f, 0.5f);
    dx_ = dest.first;
    dz_ = dest.second;
}

void Bear::update(float dt) {
    timer_ += dt;

    // Every few seconds, pick a new random direction to wander in.
    if (timer_ >= 5.0f) {
        auto dest = rnd::uniform2(-0.5f, 0.5f);
        dx_ = dest.first;
        dz_ = dest.second;
        timer_ = 0.0f;
    }


    //
    const float speed = 100.0f;
    object_->position.x += dx_ * dt * speed;
    object_->position.z += dz_ * dt * speed;

    //rotate the bear if its moving and rotate it.
    if (std::abs(dx_) > 0.01f || std::abs(dz_) > 0.01f) {
        object_->rotation.y = std::atan2(dx_, dz_);
    }
}

// createBears return type is std::vector<Bear>
std::vector<Bear> createBears(Scene& scene, int count, const std::string& modelPath, float scale) {
    AssimpLoader loader;
    auto model = loader.load(modelPath);

    //creates an empty vector of bears
    std::vector<Bear> bears;

    // Returns bears if model fails to load
    if (!model) return bears;

    // The for loop that spawns the bears
    for (int i = 0; i < count; ++i) {
        auto mesh = (i==0) ? model : model->clone();
        auto pos = rnd::uniform2(-1000.f, 1000.f);

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