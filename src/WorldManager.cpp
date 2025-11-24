#include "WorldManager.hpp"
#include <random>
#include <stdexcept>
#include <threepp/objects/Sky.hpp>
#include <threepp/materials/ShaderMaterial.hpp> // ensure ShaderMaterial visible
#include <threepp/geometries/SphereGeometry.hpp>
#include <threepp/materials/MeshBasicMaterial.hpp>
#include "Random.hpp"


namespace {
std::pair<float, float> randomPos(float min = -2000.0f, float max = 2000.f) {
    return rnd::uniform2(min, max);
}
}

WorldManager::WorldManager(threepp::Scene& scene, CollisionDetection& collisionDetection,
                           threepp::AssimpLoader& loader, float& blurLevel)
    : scene_(scene), collisionDetection_(collisionDetection), loader_(loader), blurLevel_(blurLevel) {}

void WorldManager::spawnWorld(const WorldConfig& config) {
    lastConfig_ = config; // cache for resets
    spawnMushrooms(config);
    spawnTrees(config); // trees considered static; not removed on reset
    spawnBears(config);
    spawnWolvesDefault();
}

// Removal helpers
void WorldManager::removeMushrooms() {
    for (auto& m : mushrooms_) {
        if (m && m->getModel()->parent) {
            m->removeFromScene(scene_);
        }
    }
    mushrooms_.clear();
}

void WorldManager::removeBears() {
    for (auto& b : bears_) {
        if (b.object && b.object->parent) {
            b.object->parent->remove(*b.object);
        }
    }
    bears_.clear();
}

void WorldManager::removeWolves() {
    for (auto& w : wolves_) {
        if (w.object && w.object->parent) {
            w.object->parent->remove(*w.object);
        }
    }
    wolves_.clear();
}

void WorldManager::resetDynamicEntities() {
    removeMushrooms();
    removeBears();
    removeWolves();
    // Deactivate previous dynamic colliders so they won't register collisions
    collisionDetection_.removeCollidersOfType(ColliderType::MUSHROOM);
    collisionDetection_.removeCollidersOfType(ColliderType::BEAR);
    collisionDetection_.removeCollidersOfType(ColliderType::WOLF);
    collisionDetection_.cleanupInactive();

    registeredBullets_.clear();
    blurLevel_ = 0.0f;

    spawnMushrooms(lastConfig_);
    spawnBears(lastConfig_);
    spawnWolvesDefault();

    for (auto& w : wolves_) {
        w.hits = 0;
        w.alive = true;
        w.velocity.set(0,0,0);
    }
    for (auto& b : bears_) b.timer = 0.0f;
}

void WorldManager::registerCar(const std::shared_ptr<threepp::Object3D>& car, float radius,
                               std::function<void()> onWolfCollision) {
    car_ = car;
    onWolfCollision_ = std::move(onWolfCollision);
    collisionDetection_.addCollider(car_, radius, ColliderType::CAR,
        [this](const Collider& self, const Collider& other){
            if (other.type == ColliderType::WOLF) {
                // Notify game that a wolf melee hit occurred
                if (onWolfCollision_) onWolfCollision_();
            }
        });
}

void WorldManager::setBearCollisionHandler(std::function<void()> handler) {
    onBearCollision_ = std::move(handler);
}

void WorldManager::setMushroomCollisionHandler(std::function<void()> handler) {
    onMushroomCollision_ = std::move(handler);
}

void WorldManager::spawnMushrooms(const WorldConfig& config) {
    auto mushroomTemplate = loader_.load("../Models/mushrooms/model.obj");
    if (!mushroomTemplate) throw std::runtime_error("Failed to load mushroom model");

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

        collisionDetection_.addCollider(mushroomClone, config.mushroomCollisionRadius, ColliderType::MUSHROOM,
            [this](const Collider& self, const Collider& other){
                if (other.type == ColliderType::CAR) {
                    for (auto it = mushrooms_.begin(); it != mushrooms_.end(); ++it) {
                        if ((*it)->getModel().get() == self.object.get()) {
                            (*it)->handleCollision();
                            (*it)->removeFromScene(scene_);
                            mushrooms_.erase(it);

                            // Notify Game that mushroom was collected
                            if (onMushroomCollision_) onMushroomCollision_();
                            break;
                        }
                    }
                }
            });

        scene_.add(mushroom->getModel());
        mushrooms_.push_back(mushroom);
    }
}

void WorldManager::spawnTrees(const WorldConfig& config) {
    auto treeTemplate = loader_.load("../Models/model.obj");
    if (!treeTemplate) throw std::runtime_error("Failed to load tree model");

    std::vector<threepp::Vector3> treePositions;
    treePositions.reserve(config.treeCount);
    trees_.reserve(config.treeCount);

    for (int i = 0; i < config.treeCount; ++i) {
        auto [x, z] = randomPos();
        auto treeClone = treeTemplate->children[0]->clone();
        treeClone->position.set(x, 65, z);
        treeClone->scale.set(config.treeScale, config.treeScale, config.treeScale);

        auto tree = std::make_shared<GameObject>(treeClone, config.treeCollisionRadius, GameObjectType::TREE);
        scene_.add(tree->getModel());
        trees_.push_back(tree);
        treePositions.emplace_back(x, 60, z);

        collisionDetection_.addCollider(treeClone, config.treeCollisionRadius, ColliderType::TREE);
    }

    collisionDetection_.setObstacles(treePositions);
}

void WorldManager::spawnBears(const WorldConfig& config) {
    ::spawnBears(scene_, bears_, config.bearCount, "../Models/Mesh_Bear.obj", config.bearScale);

    for (auto& b : bears_) {
        collisionDetection_.addCollider(b.object, config.bearCollisionRadius, ColliderType::BEAR,
            [this](const Collider& bear, const Collider& other){
                if (other.type == ColliderType::CAR && onBearCollision_) {
                    onBearCollision_();
                }
            });
    }
}

void WorldManager::spawnWolvesDefault() {
    // Calculate wolf speed based on difficulty level
    // Level 1: 40 base speed, 150 acceleration
    // Level 2: 70 base speed, 200 acceleration
    // Level 3: 100 base speed, 250 acceleration
    float baseSpeed = 50.0f + (difficultyLevel_ - 1) * 40.0f;
    float baseAcceleration = 150.0f + (difficultyLevel_ - 1) * 50.0f;

    spawnWolves(scene_, wolves_, 3, "../Models/wolf/model.obj", 5.0f, baseSpeed, baseAcceleration);
    for (auto& w : wolves_) {
        collisionDetection_.addCollider(w.object, 2.0f, ColliderType::WOLF);
    }
}

void WorldManager::setDifficultyLevel(int level) {
    if (level >= 1 && level <= 3) {
        difficultyLevel_ = level;
    }
}

void WorldManager::syncBullets(const std::vector<std::shared_ptr<class Bullet>>& bullets) {
    for (auto& b : bullets) {
        auto* ptr = b->getModel().get();
        if (registeredBullets_.insert(ptr).second) {
            collisionDetection_.addCollider(b->getModel(), 5.0f, ColliderType::BULLET,
                [this](const Collider& bullet, const Collider& other){
                    if (other.type != ColliderType::WOLF) return;
                    for (auto& w : wolves_) {
                        if (w.object.get() == other.object.get() && w.alive) {
                            w.hits++;
                            w.maxSpeed += 50.0f;
                            if (w.hits >= 5) {
                                w.alive = false;
                                w.object->visible = false;
                                w.velocity.set(0,0,0);
                            }
                            break;
                        }
                    }
                    bullet.object->visible = false;
                });
        }
    }
}

void WorldManager::update(float dt, const threepp::Vector3& carPos,
                          const std::vector<std::shared_ptr<class Bullet>>& bullets) {
    syncBullets(bullets);
    updateWolves(wolves_, dt, carPos);
    updateBears(bears_, dt);

    collisionDetection_.processAllPairs();
}

void WorldManager::addSkyAndSun(float elevationDeg, float azimuthDeg) {
    using namespace threepp;

    sky_ = Sky::create();
    sky_->scale.set(50000, 50000, 50000);
    sky_->frustumCulled = false; // ensure sky never gets culled

    auto shaderMat = sky_->material()->as<ShaderMaterial>();
    if (shaderMat) {
        const float DegToRad = math::PI / 180.0f;
        float elev = elevationDeg * DegToRad;
        float azim = azimuthDeg * DegToRad;
        Vector3 sunPos(std::cos(elev) * std::sin(azim), std::sin(elev), std::cos(elev) * std::cos(azim));
        shaderMat->uniforms["sunPosition"] = Uniform(sunPos);
        // Do not override turbidity/rayleigh/mieCoefficients: keep Sky defaults
    }

    scene_.add(sky_);

    // Directional sunlight aligned with the shader sun direction
    Vector3 dir(0, 1, 0);
    if (shaderMat) {
        dir = shaderMat->uniforms["sunPosition"].value<Vector3>().clone().normalize();
    }
    sunLight_ = DirectionalLight::create(threepp::Color::white, 1.6f);
    sunLight_->position.copy(dir.multiplyScalar(10000.0f));
    scene_.add(sunLight_);
}

void WorldManager::updateSkyPosition(const threepp::Vector3& cameraPos) {
    if (sky_) {
        threepp::Vector3 p = cameraPos;
        p.y += skyYOffset_;
        sky_->position.copy(p);
    }
    if (sunLight_) {
        // keep as distant directional light (no need to move with camera)
    }
}

const std::vector<std::shared_ptr<GameObject>>& WorldManager::getTrees() const { return trees_; }
const std::vector<Wolf>& WorldManager::getWolves() const { return wolves_; }
bool WorldManager::allWolvesDead() const {
    for (const auto& w : wolves_) { if (w.alive) return false; }
    return true;
}