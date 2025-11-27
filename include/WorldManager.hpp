#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP

#include <threepp/threepp.hpp>
#include <threepp/objects/Sky.hpp>
#include <threepp/lights/DirectionalLight.hpp>
#include <memory>
#include <vector>
#include <unordered_set>
#include <threepp/loaders/AssimpLoader.hpp>

#include "GameObject.hpp"
#include "CollisionDetection.hpp"
#include "Wolf.hpp"
#include "bears.hpp"
#include "bullet.hpp"

struct WorldConfig {
    int treeCount = 2000;
    int mushroomCount = 600;
    int bearCount = 600;

    float treeScale = 40.0f;
    float mushroomScale = 20.0f;
    float bearScale = 0.1f;

    float treeCollisionRadius = 7.0f;
    float mushroomCollisionRadius = 4.0f;
    float bearCollisionRadius = 7.0f;
};

class WorldManager {
public:
    WorldManager(threepp::Scene& scene, CollisionDetection& collisionDetection,
                 threepp::AssimpLoader& loader, float& blurLevel);

    void spawnWorld(const WorldConfig& config);

    // Register car and hook a game-level callback for wolf collisions (end game)
    void registerCar(const std::shared_ptr<threepp::Object3D>& car, float radius,
                     std::function<void()> onWolfCollision);

    // Let Game provide behavior when a bear collides with the car (e.g., slow down with cooldown)
    void setBearCollisionHandler(std::function<void()> handler);

    // Let Game provide behavior when mushroom is collected (e.g., play sound, apply blur)
    void setMushroomCollisionHandler(std::function<void()> handler);

    // Called per frame: updates wolves, bears, syncs bullets, processes all collisions
    void update(float dt, const threepp::Vector3& carPos,
                const std::vector<std::shared_ptr<class Bullet>>& bullets);

    const std::vector<std::shared_ptr<GameObject>>& getTrees() const;
    const std::vector<Wolf>& getWolves() const;
    const std::vector<std::shared_ptr<GameObject>>& getMushrooms() const { return mushrooms_; }
    const std::vector<MovingBear>& getBears() const { return bears_; }
    bool allWolvesDead() const;

    void setDifficultyLevel(int level); // Set difficulty (1-3) for wolf spawn speed

    void addSkyAndSun(float elevationDeg =15.0f, float azimuthDeg = 180.0f);
    void updateSkyPosition(const threepp::Vector3& cameraPos); // new: keep camera inside sky
    void setSkyYOffset(float yOffset) { skyYOffset_ = yOffset; }
    void createGround(float groundsize);

    // Restart support: remove & respawn dynamic entities (mushrooms, bears, wolves) & reset blur
    void resetDynamicEntities();

private:
    void spawnMushrooms(const WorldConfig& config);
    void spawnTrees(const WorldConfig& config);
    void spawnBears(const WorldConfig& config);
    void spawnWolvesDefault();

    void syncBullets(const std::vector<std::shared_ptr<class Bullet>>& bullets);

    // Removal helpers
    void removeMushrooms();
    void removeBears();
    void removeWolves();

    WorldConfig lastConfig_{}; // store last used config for resets

private:
    threepp::Scene& scene_;
    CollisionDetection& collisionDetection_;
    threepp::AssimpLoader& loader_;
    float& blurLevel_;

    std::vector<std::shared_ptr<GameObject>> trees_;
    std::vector<std::shared_ptr<GameObject>> mushrooms_;
    std::vector<Wolf> wolves_;
    std::vector<MovingBear> bears_;

    std::shared_ptr<threepp::Object3D> car_;
    std::shared_ptr<threepp::Sky> sky_; // store sky to move with camera
    std::shared_ptr<threepp::DirectionalLight> sunLight_; // store sunlight for future tweaks
    std::shared_ptr<threepp::Mesh> ground_; // store ground mesh for shadow receiving
    float skyYOffset_ = -1300.0f;


    std::function<void()> onWolfCollision_;
    std::function<void()> onBearCollision_;
    std::function<void()> onMushroomCollision_;

    std::unordered_set<const threepp::Object3D*> registeredBullets_;

    int difficultyLevel_ = 1; // Difficulty level (1-3) affects wolf speed
};

#endif // WORLD_MANAGER_HPP
