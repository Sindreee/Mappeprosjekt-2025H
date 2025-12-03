#include "WorldManager.hpp"
#include "GameConstants.hpp"
#include "Random.hpp"
#include <threepp/materials/ShaderMaterial.hpp>
#include <threepp/geometries/PlaneGeometry.hpp>
#include <threepp/materials/MeshPhongMaterial.hpp>
#include <threepp/lights/DirectionalLight.hpp>
#include <algorithm>
#include <cmath>

using namespace threepp;

WorldManager::WorldManager(Scene& scene, CollisionDetection& collisionDetection,
                           AssimpLoader& loader, float& blurLevel)
    : scene_(scene), collisionDetection_(collisionDetection), loader_(loader), blurLevel_(blurLevel) {}

// Spawns all the independent world entities based on the provided configuration except for car
void WorldManager::spawnWorld(const WorldConfig& config) {
    lastConfig_ = config;
    if (!ground_) createGround(Config::WORLD_SIZE);
    spawnTrees(config);
    spawnMushrooms(config);
    spawnBears(config);
    spawnWolvesDefault();
}


// Update entities (wolves, bears), sync active bullets into the collision system,
// then process all collision pairs for this frame.
void WorldManager::update(float dt, const Vector3& carPos, const std::vector<std::shared_ptr<Bullet>>& bullets) {
    for (auto& wolf : wolves_) wolf.update(dt, carPos);
    for (auto& bear : bears_) bear.update(dt);
    syncBullets(bullets);
    collisionDetection_.processAllPairs();
}

void WorldManager::spawnTrees(const WorldConfig& config) {
    auto model = loader_.load("Models/model.obj");
    if (!model) return;

    trees_.reserve(config.treeCount);

    // Spawn trees at random positions within world bounds
    for (int i = 0; i < config.treeCount; ++i) {
        auto pos = rnd::uniform2(-Config::WORLD_BOUNDS, Config::WORLD_BOUNDS);
        auto clone = model->children[0]->clone();
        clone->position.set(pos.first, 65, pos.second);
        clone->scale.set(config.treeScale, config.treeScale, config.treeScale);

        //Give trees shadows
        clone->traverseType<Mesh>([](Mesh& m){
            m.castShadow = true;
            m.receiveShadow = true;
        });

        auto tree = std::make_shared<GameObject>(clone);
        scene_.add(tree->getModel());
        trees_.push_back(tree);

        // Register tree collider
        collisionDetection_.addCollider(clone, config.treeCollisionRadius, ColliderType::TREE);
    }
}

// Spawn bears into the scene and register a collision handler so bear–car hits trigger bear logic
void WorldManager::spawnBears(const WorldConfig& config) {
    bears_ = createBears(scene_, config.bearCount, "Models/Mesh_Bear.obj", config.bearScale);
    for (auto& b : bears_) {

        collisionDetection_.addCollider(b.getModel(), config.bearCollisionRadius, ColliderType::BEAR,
            [this](const Collider&, const Collider& other){
                this->handleBearLogic(other);
            });
    }
}


// Handle a bear collision by triggering the car hit callback when the other collider is the car
void WorldManager::handleBearLogic(const Collider& otherCollider) {
    if (otherCollider.type == ColliderType::CAR && onBearCollision_) {
        onBearCollision_();
    }
}

// Spawn the "pack" of wolves based on the current difficulty and register their colliders
void WorldManager::spawnWolvesDefault() {
    wolves_ = createWolves(scene_, Config::WOLF_COUNT_BASE, "Models/wolf/model.obj", (float)difficultyLevel_);
    for (auto& w : wolves_) {
        collisionDetection_.addCollider(w.getModel(), 2.0f, ColliderType::WOLF);
    }
}


void WorldManager::spawnMushrooms(const WorldConfig& config) {
    auto model = loader_.load("Models/mushrooms/model.obj");
    if (!model) return;

    mushrooms_.reserve(config.mushroomCount);

    for (int i = 0; i < config.mushroomCount; ++i) {
        auto pos = rnd::uniform2(-Config::WORLD_BOUNDS, Config::WORLD_BOUNDS);

        // Clone the base mushroom mesh so each instance has its own transform.
        auto clone = model->children[0]->clone();
        clone->position.set(pos.first, -4, pos.second);
        clone->scale.set(config.mushroomScale, config.mushroomScale, config.mushroomScale);

        auto gameObj = std::make_shared<GameObject>(clone);

        // When this mushroom is hit, increase blur level (clamped to MAX_BLUR).
        gameObj->setOnCollision([this](GameObject&) {
            blurLevel_ = std::min(blurLevel_ + 3.0f, Config::MAX_BLUR);
        });

        // Register a collider that routes mushroom hits to handleMushroomLogic
        collisionDetection_.addCollider(clone, config.mushroomCollisionRadius, ColliderType::MUSHROOM,
            [this](const Collider& self, const Collider& other){
                this->handleMushroomLogic(self, other);
            });

        scene_.add(gameObj->getModel());
        mushrooms_.push_back(gameObj);
    }
}

// When a mushroom collides with the car apply blur and speed increase, remove it from the scene,
// and remove the global mushroom collision callback
void WorldManager::handleMushroomLogic(const Collider& mushroomCollider, const Collider& otherCollider) {
    if (otherCollider.type != ColliderType::CAR) return;


    for (auto it = mushrooms_.begin(); it != mushrooms_.end(); ++it) {

        if ((*it)->getModel().get() == mushroomCollider.object.get()) {

            (*it)->handleCollision();      // Trigger the blur effect
            (*it)->removeFromScene(scene_); // remove graphics
            mushrooms_.erase(it);           // erase from list
            if (onMushroomCollision_) onMushroomCollision_();
            break;
        }
    }
}

// Synchronize bullet colliders with active entities
void WorldManager::syncBullets(const std::vector<std::shared_ptr<Bullet>>& bullets) {
    for (const auto& bullet : bullets) {
        if (!bullet->isAlive()) continue;

        auto* ptr = bullet->getModel().get();
        if (registeredBullets_.insert(ptr).second) {
            collisionDetection_.addCollider(bullet->getModel(), 5.0f, ColliderType::BULLET,
                [this](const Collider& b, const Collider& other){

                    // check if the bullet is still visible
                    if (!b.object->visible) return;

                    if (other.type == ColliderType::WOLF) {
                        handleBulletWolfCollision(other.object.get());
                        b.object->visible = false; // make the bullet invisible and mark it as used
                    }
                });
        }
    }
}


// Find the wolf whose model matches the hit mesh and apply one hit of damage to it
void WorldManager::handleBulletWolfCollision(threepp::Object3D* wolfMesh) {
    for (auto& w : wolves_) {
        if (w.getModel().get() == wolfMesh && w.isAlive()) {
            w.takeDamage();
            break;
        }
    }
}

// Create the ground_ plane and enable it to receive shadows
void WorldManager::createGround(float groundsize) {
    auto geometry = PlaneGeometry::create(groundsize, groundsize);
    geometry->rotateX(-math::PI / 2);
    auto material = MeshPhongMaterial::create();
    material->color = Color::forestgreen;
    ground_ = Mesh::create(geometry, material);
    ground_->position.y = -4.4f;
    ground_->receiveShadow = true;
    scene_.add(ground_);
}


// Add a large sky dome with a shader material that simulates atmospheric scattering
// Most of this code is adapted from threepp examples found with DeepWiki
void WorldManager::addSkyAndSun(float elevationDeg, float azimuthDeg) {
    sky_ = Sky::create();
    sky_->scale.set(50000, 50000, 50000);
    auto shaderMat = sky_->material()->as<ShaderMaterial>();
    if (shaderMat) {
        float elev = math::degToRad(elevationDeg);
        float azim = math::degToRad(azimuthDeg);
        Vector3 sunPos(std::cos(elev) * std::sin(azim), std::sin(elev), std::cos(elev) * std::cos(azim));
        shaderMat->uniforms["sunPosition"] = Uniform(sunPos);

        auto sunLight = DirectionalLight::create(Color::lightyellow, 1.7f);
        sunLight->position.copy(sunPos.clone().normalize().multiplyScalar(5000.0f));
        sunLight->castShadow = true;

        sunLight->shadow->mapSize.set(4096, 4096);
        sunLight->shadow->bias = -0.0001f;

        float viewSize = 2500.0f;
        sunLight->shadow->camera = std::make_unique<OrthographicCamera>(
            -viewSize, viewSize, viewSize, -viewSize, 0.5f, 10000.0f
        );
        sunLight->shadow->camera->updateProjectionMatrix();

        scene_.add(sunLight);
    }
    scene_.add(sky_);
}

// Keep the sky dome centered on the camera
void WorldManager::updateSkyPosition(const Vector3& cameraPos) {
    if (sky_) {
        Vector3 p = cameraPos;
        p.y += skyYOffset_;
        sky_->position.copy(p);
    }
}

// Clear all dynamic entities and their colliders, reset blur/bullet state,
// then respawn mushrooms, bears, and wolves using the last world config
void WorldManager::resetDynamicEntities() {
    removeMushrooms();
    removeBears();
    removeWolves();

    collisionDetection_.removeCollidersOfType(ColliderType::MUSHROOM);
    collisionDetection_.removeCollidersOfType(ColliderType::BEAR);
    collisionDetection_.removeCollidersOfType(ColliderType::WOLF);
    collisionDetection_.cleanupInactive();

    registeredBullets_.clear();
    blurLevel_ = 0.0f;

    spawnMushrooms(lastConfig_);
    spawnBears(lastConfig_);
    spawnWolvesDefault();
}

// Remove all mushrooms, bears, and wolves from the scene and clear their lists
void WorldManager::removeMushrooms() {
    for(auto& m : mushrooms_) if(m) m->removeFromScene(scene_);
    mushrooms_.clear();
}
void WorldManager::removeBears() {
    for(auto& b : bears_) if(b.getModel() && b.getModel()->parent) b.getModel()->parent->remove(*b.getModel());
    bears_.clear();
}
void WorldManager::removeWolves() {
    for(auto& w : wolves_) if(w.getModel() && w.getModel()->parent) w.getModel()->parent->remove(*w.getModel());
    wolves_.clear();
}

// Accessors
const std::vector<Wolf>& WorldManager::getWolves() const { return wolves_; }
const std::vector<std::shared_ptr<GameObject>>& WorldManager::getTrees() const { return trees_; }

// Returns true if no wolves are still alive
bool WorldManager::allWolvesDead() const {
    for(const auto& w : wolves_) if(w.isAlive()) return false;
    return true;
}

void WorldManager::setDifficultyLevel(int level) {
    difficultyLevel_ = std::clamp(level, 1, 3);
}

// Register the car object with a collider that triggers the provided callback on wolf collisions
void WorldManager::registerCar(const std::shared_ptr<threepp::Object3D>& car, float radius, std::function<void()> onWolfCollision) {
    collisionDetection_.addCollider(car, radius, ColliderType::CAR,
        [onWolfCollision](const Collider&, const Collider& other){
            if (other.type == ColliderType::WOLF && onWolfCollision) onWolfCollision();
        });
}

// Set the bear and mushroom collision callbacks
void WorldManager::setBearCollisionHandler(std::function<void()> handler) { onBearCollision_ = std::move(handler); }
void WorldManager::setMushroomCollisionHandler(std::function<void()> handler) { onMushroomCollision_ = std::move(handler); }