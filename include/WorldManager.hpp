#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>
#include "GameObject.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include "CollisionDetection.hpp"
#include "Wolf.hpp"
#include "car.hpp"

struct WorldConfig {
    int treeCount = 1000;
    int mushroomCount = 500;
    float treeScale = 40.0f;
    float mushroomScale = 20.0f;
    float treeCollisionRadius = 7.0f;
    float mushroomCollisionRadius = 15.0f;
};

class WorldManager {
public:
    WorldManager(threepp::Scene& scene, CollisionDetection& collisionDetection,
                 threepp::AssimpLoader& loader, float& blurLevel);

    void spawnWorld(const WorldConfig& config);
    void update(float dt, const threepp::Vector3& carPos);
    const std::vector<std::shared_ptr<GameObject>>& getTrees() const;
    void processMushroomCollisions(const threepp::Vector3& carPos);
    const std::vector<Wolf>& getWolves() const; // new accessor

private:
    threepp::Scene& scene_;
    CollisionDetection& collisionDetection_;
    threepp::AssimpLoader& loader_;
    float& blurLevel_;

    std::vector<std::shared_ptr<GameObject>> trees_;
    std::vector<std::shared_ptr<GameObject>> mushrooms_;
    std::vector<Wolf> wolves_;
};

#endif // WORLD_MANAGER_HPP
