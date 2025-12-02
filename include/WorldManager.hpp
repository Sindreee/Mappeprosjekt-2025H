#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP

#include <threepp/threepp.hpp>
#include <threepp/objects/Sky.hpp>
#include <memory>
#include <vector>
#include <unordered_set>
#include <threepp/loaders/AssimpLoader.hpp>

#include "GameObject.hpp"
#include "CollisionDetection.hpp"
#include "Wolf.hpp"
#include "Bear.hpp"
#include "bullet.hpp"

// Configuration values used when (re)spawning the world contents.
struct WorldConfig {
    int treeCount = 500;
    int mushroomCount = 150;
    int bearCount = 200;
    float treeScale = 40.0f;
    float mushroomScale = 20.0f;
    float bearScale = 0.1f;
    float treeCollisionRadius = 7.0f;
    float mushroomCollisionRadius = 4.0f;
    float bearCollisionRadius = 7.0f;
};

// Owns and manages all environment objects (trees, mushrooms, bears, wolves, sky, ground).
// Integrates with the collision system and exposes callbacks for gameplay events.
class WorldManager {
public:
    WorldManager(threepp::Scene& scene, CollisionDetection& collisionDetection,
                 threepp::AssimpLoader& loader, float& blurLevel);

    // Create static and dynamic world entities based on the given configuration.
    void spawnWorld(const WorldConfig& config);

    // Per-frame update for AI-driven entities and collision processing.
    void update(float dt, const threepp::Vector3& carPos, const std::vector<std::shared_ptr<class Bullet>>& bullets);

    // Register the player's car as a collider; onWolfCollision is invoked when a wolf hits the car.
    void registerCar(const std::shared_ptr<threepp::Object3D>& car, float radius, std::function<void()> onWolfCollision);

    // Set callbacks fired when the car collides with a bear or mushroom.
    void setBearCollisionHandler(std::function<void()> handler);
    void setMushroomCollisionHandler(std::function<void()> handler);

    const std::vector<std::shared_ptr<GameObject>>& getTrees() const;
    const std::vector<Wolf>& getWolves() const;
    const std::vector<Bear>& getBears() const { return bears_; }

    // Returns true if no wolves are still alive.
    bool allWolvesDead() const;

    // Clamp and store the current difficulty level (affects wolf stats on spawn).
    void setDifficultyLevel(int level);

    // Add a procedural sky and directional sun light to the scene.
    void addSkyAndSun(float elevationDeg = 35.0f, float azimuthDeg = 180.0f);

    // Keep the sky dome centered on the camera, with an adjustable vertical offset.
    void updateSkyPosition(const threepp::Vector3& cameraPos);
    void setSkyYOffset(float yOffset) { skyYOffset_ = yOffset; }

    // Create a simple colored ground plane.
    void createGround(float groundsize);

    // Remove and respawn dynamic entities (mushrooms, bears, wolves) and related colliders.
    void resetDynamicEntities();

private:
    void spawnMushrooms(const WorldConfig& config);
    void spawnTrees(const WorldConfig& config);
    void spawnBears(const WorldConfig& config);
    void spawnWolvesDefault();

    void syncBullets(const std::vector<std::shared_ptr<class Bullet>>& bullets);
    void handleBulletWolfCollision(threepp::Object3D* wolfMesh);


    void handleMushroomLogic(const Collider& mushroomCollider, const Collider& otherCollider);
    void handleBearLogic(const Collider& otherCollider);

    void removeMushrooms();
    void removeBears();
    void removeWolves();

    threepp::Scene& scene_;
    CollisionDetection& collisionDetection_;
    threepp::AssimpLoader& loader_;
    float& blurLevel_;
    WorldConfig lastConfig_{}; // Last used configuration so we can respawn with the same settings

    std::vector<std::shared_ptr<GameObject>> trees_;
    std::vector<std::shared_ptr<GameObject>> mushrooms_;
    std::vector<Wolf> wolves_;
    std::vector<Bear> bears_;

    std::shared_ptr<threepp::Sky> sky_;
    std::shared_ptr<threepp::Mesh> ground_;
    float skyYOffset_ = -1300.0f;

    std::function<void()> onBearCollision_;
    std::function<void()> onMushroomCollision_;
    std::unordered_set<const threepp::Object3D*> registeredBullets_;

    int difficultyLevel_ = 1;
};

#endif

