#include "WorldManager.hpp"
#include <random>
#include <stdexcept>
#include "bears.hpp"


namespace {
std::pair<float, float> randomPos(float min = 0.f, float max = 4000.f) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return {dist(gen), dist(gen)};
}
}

WorldManager::WorldManager(threepp::Scene& scene, CollisionDetection& collisionDetection,
                           threepp::AssimpLoader& loader, float& blurLevel)
    : scene_(scene), collisionDetection_(collisionDetection), loader_(loader), blurLevel_(blurLevel) {}

void WorldManager::spawnWorld(const WorldConfig& config) {
    auto mushroomTemplate = loader_.load("../Models/mushrooms/model.obj");
    if (!mushroomTemplate) {
        throw std::runtime_error("Failed to load mushroom model");
    }

    mushrooms_.reserve(config.mushroomCount);
    for (int i = 0; i < config.mushroomCount; ++i) {
        auto [x, z] = randomPos();
        auto mushroomClone = mushroomTemplate->children[0]->clone();
        mushroomClone->position.set(x, -5, z);
        mushroomClone->scale.set(config.mushroomScale, config.mushroomScale, config.mushroomScale);

        auto mushroom = std::make_shared<GameObject>(mushroomClone, config.mushroomCollisionRadius, GameObjectType::MUSHROOM);
        mushroom->setOnCollision([this](GameObject&) {
            blurLevel_ = std::min(blurLevel_ + 3.0f, 8.0f);
        });

        scene_.add(mushroom->getModel());
        mushrooms_.push_back(mushroom);
    }

    // Load tree template once
    auto treeTemplate = loader_.load("../Models/model.obj");
    if (!treeTemplate) {
        throw std::runtime_error("Failed to load tree model");
    }

    std::vector<threepp::Vector3> treePositions;
    treePositions.reserve(config.treeCount);

    trees_.reserve(config.treeCount);
    for (int i = 0; i < config.treeCount; ++i) {
        auto [x, z] = randomPos();

        // Clone the template instead of loading from file
        auto treeClone = treeTemplate->children[0]->clone();
        treeClone->position.set(x, 65, z);
        treeClone->scale.set(config.treeScale, config.treeScale, config.treeScale);

        auto tree = std::make_shared<GameObject>(treeClone, config.treeCollisionRadius, GameObjectType::TREE);

        scene_.add(tree->getModel());
        trees_.push_back(tree);
        treePositions.emplace_back(x, 60, z);
    }

    collisionDetection_.setObstacles(treePositions);

    // Spawn 3 wolves with acceleration
    // maxSpeed = 40.0f, acceleration = 30.0f (so deer can outrun them with sharp turns)
    spawnWolves(scene_, wolves_, 3, "../Models/wolf/model.obj", 5.0f, 50.0f, 30.0f);
}

void WorldManager::processMushroomCollisions(const threepp::Vector3& carPos) {
    for (auto it = mushrooms_.begin(); it != mushrooms_.end();) {
        if ((*it)->checkCollision(carPos)) {
            (*it)->handleCollision();
            (*it)->removeFromScene(scene_);
            it = mushrooms_.erase(it);
        } else {
            ++it;
        }
    }
}



void WorldManager::update(float dt, const threepp::Vector3& carPos) {
    // Update wolves to chase the car/deer
    updateWolves(wolves_, dt, carPos);
    (void)dt;

}

const std::vector<std::shared_ptr<GameObject>>& WorldManager::getTrees() const {
    return trees_;
}

const std::vector<Wolf>& WorldManager::getWolves() const {
    return wolves_;
}
