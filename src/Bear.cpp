#include "Bear.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include "Random.hpp"

using namespace threepp;

Bear::Bear(std::shared_ptr<Object3D> mesh) : object_(std::move(mesh)) {
    auto dest = rnd::uniform2(-0.5f, 0.5f);
    dx_ = dest.first;
    dz_ = dest.second;
}

void Bear::update(float dt) {
    timer_ += dt;
    if (timer_ >= 5.0f) {
        auto dest = rnd::uniform2(-0.5f, 0.5f);
        dx_ = dest.first;
        dz_ = dest.second;
        timer_ = 0.0f;
    }

    float speed = 100.0f;
    object_->position.x += dx_ * dt * speed;
    object_->position.z += dz_ * dt * speed;

    if (std::abs(dx_) > 0.01f || std::abs(dz_) > 0.01f) {
        object_->rotation.y = std::atan2(dx_, dz_);
    }
}

std::vector<Bear> createBears(Scene& scene, int count, const std::string& modelPath, float scale) {
    AssimpLoader loader;
    auto model = loader.load(modelPath);
    std::vector<Bear> bears;

    if (!model) return bears;

    for (int i = 0; i < count; ++i) {
        auto mesh = (i==0) ? model : model->clone();
        auto pos = rnd::uniform2(-1600.f, 1600.f);

        mesh->position.set(pos.first, 2, pos.second);
        mesh->scale.set(scale, scale, scale);

        mesh->traverseType<Mesh>([](Mesh& m){
            m.castShadow = true;
            m.receiveShadow = true;
        });

        scene.add(mesh);
        bears.emplace_back(mesh);
    }
    return bears;
}