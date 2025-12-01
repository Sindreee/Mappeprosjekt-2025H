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
    int treeCount = 1000;
    int mushroomCount = 300;
    int bearCount = 400;

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

    void registerCar(const std::shared_ptr<threepp::Object3D>& car, float radius,
                     std::function<void()> onWolfCollision);

    void setBearCollisionHandler(std::function<void()> handler);
    void setMushroomCollisionHandler(std::function<void()> handler);

    void handleBulletWolfCollision(threepp::Object3D* wolfMesh);

    void update(float dt, const threepp::Vector3& carPos,
                const std::vector<std::shared_ptr<class Bullet>>& bullets);

    const std::vector<std::shared_ptr<GameObject>>& getTrees() const;
    const std::vector<Wolf>& getWolves() const;
    const std::vector<std::shared_ptr<GameObject>>& getMushrooms() const { return mushrooms_; }
    const std::vector<MovingBear>& getBears() const { return bears_; }
    bool allWolvesDead() const;

    void setDifficultyLevel(int level);

    // Reverted to standard manual addition
    void addSkyAndSun(float elevationDeg = 35.0f, float azimuthDeg = 180.0f);

    void updateSkyPosition(const threepp::Vector3& cameraPos);
    void setSkyYOffset(float yOffset) { skyYOffset_ = yOffset; }
    void createGround(float groundsize);

    void resetDynamicEntities();

private:
    void spawnMushrooms(const WorldConfig& config);
    void spawnTrees(const WorldConfig& config);
    void spawnBears(const WorldConfig& config);
    void spawnWolvesDefault();

    void syncBullets(const std::vector<std::shared_ptr<class Bullet>>& bullets);

    void removeMushrooms();
    void removeBears();
    void removeWolves();

    WorldConfig lastConfig_{};

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
    std::shared_ptr<threepp::Sky> sky_;
    std::shared_ptr<threepp::DirectionalLight> sunLight_;

    std::shared_ptr<threepp::Mesh> ground_;
    float skyYOffset_ = -1300.0f;

    std::function<void()> onWolfCollision_;
    std::function<void()> onBearCollision_;
    std::function<void()> onMushroomCollision_;

    std::unordered_set<const threepp::Object3D*> registeredBullets_;

    int difficultyLevel_ = 1;
};

#endif // WORLD_MANAGER_HPP