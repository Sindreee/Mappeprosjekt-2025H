//
// Created by sindr on 11/17/2025.
//

#include "Game.hpp"
#include <iostream>
#include <iomanip>

using namespace threepp;

Game::Game()
    : canvas_("Mappeoppgave"),
      renderer_(canvas_.size()),
      scene_(threepp::Scene::create()),
      camera_(threepp::PerspectiveCamera::create(75, canvas_.aspect(), 0.1f, 1000)),
      collisionDetection_(7.0f),
      turret_("../Models/turretV1bla4.obj", 3.0f) {
    renderer_.setClearColor(threepp::Color::white);
    scene_->background = threepp::Color::white;

    initializeScene();
    setupLights();
    setupTurret();
    configureEvents();
}

int Game::run() {
    canvas_.animate([&]() { tick(); });
    return 0;
}

void Game::initializeScene() {
    scene_->add(car_.getGroup());

    // Register car collider first so callbacks referencing controller_ can work after controller init
    int carColliderId = collisionDetection_.addCollider(car_.getGroup(), carCollisionRadius_, ColliderType::CAR);
    (void)carColliderId;

    cameraRig_ = std::make_unique<CameraRig>();
    orbitControls_ = std::make_unique<threepp::OrbitControls>(*camera_, canvas_);
    orbitControls_->target.set(0, 0, 0);
    orbitControls_->update();

    canvas_.addKeyListener(*cameraRig_); // allow L key to reverse view

    blurOverlay_ = std::make_unique<BlurOverlay>();
    worldManager_ = std::make_unique<WorldManager>(*scene_, collisionDetection_, loader_, blurLevel_);
    worldManager_->spawnWorld({});

    controller_ = std::make_unique<KeyController>(*car_.getGroup(), collisionDetection_);
    canvas_.addKeyListener(*controller_);

    canvas_.addKeyListener(turret_);

    try {
        spawnBears(*scene_, bears_, 500, "../Models/Mesh_Bear.obj", 0.1f);
        // Register bears as colliders with slowdown callback
        for (auto& b : bears_) {
            collisionDetection_.addCollider(b.object, bearCollisionRadius_, ColliderType::BEAR,
                [this](const Collider& bear, const Collider& other){
                    if (other.type == ColliderType::CAR && bearCollisionCooldown_ <= 0.0f) {
                        controller_->adjustMaxSpeed(-15.0f);
                        if (controller_->getMaxSpeed() < 30.0f) controller_->setMaxSpeed(30.0f);
                        bearCollisionCooldown_ = 1.0f;
                    }
                });
        }
    } catch (const std::exception& e) {
        std::cerr << "spawnBears failed: " << e.what() << std::endl;
        initialized_ = false;
    }
}

void Game::setupLights() {
    auto light = threepp::DirectionalLight::create();
    light->position.set(0, 15, 0);
    scene_->add(light);

    auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.5f);
    scene_->add(ambient);
}

void Game::setupTurret() {
    turret_.setRotation(0, 0, 0);
    turret_.setPosition(0, -100.0f, 0);
    turret_.setScale(0.03f, 0.03f, 0.03f);
    turret_.attachTo(*car_.getGroup(), threepp::Vector3(0, 1.0f, 0));
}

void Game::configureEvents() {
    canvas_.onWindowResize([&](threepp::WindowSize size) {
        camera_->aspect = size.aspect();
        camera_->updateProjectionMatrix();
        renderer_.setSize(size);
    });
}

void Game::processBearCollisions(const threepp::Vector3& carPos) {
    // Deprecated: handled via CollisionDetection callbacks
    (void)carPos;
}

void Game::processWolfCollisions(const threepp::Vector3& carPos) {
    // Deprecated: handled via CollisionDetection callbacks (CAR vs WOLF)
    (void)carPos;
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ = std::max(0.0f, bearCollisionCooldown_ - dt);

    controller_->update(dt);

    car_.setSpeed(controller_->getSpeed());
    car_.setRotation(controller_->getRotation());
    car_.update(dt);

    const auto& carPos = car_.getPosition();

    // Register newly spawned bullets as colliders (one-time per bullet)
    {
        auto& bullets = const_cast<std::vector<std::shared_ptr<Bullet>>&>(turret_.getBullets());
        for (auto& b : bullets) {
            bool alreadyRegistered = false;
            for (const auto& c : collisionDetection_.colliders()) {
                if (c.type == ColliderType::BULLET && c.object.get() == b->getModel().get()) { alreadyRegistered = true; break; }
            }
            if (!alreadyRegistered) {
                collisionDetection_.addCollider(b->getModel(), 5.0f, ColliderType::BULLET,
                    [this](const Collider& bullet, const Collider& other){
                        if (other.type == ColliderType::WOLF) {
                            // Increment wolf state & expire bullet
                            for (auto& w : const_cast<std::vector<Wolf>&>(worldManager_->getWolves())) {
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
                            // Deactivate bullet collider
                            bullet.object->visible = false;
                        }
                    });
            }
        }
    }

    // Process generic collisions
    collisionDetection_.processAllPairs();

    // Victory check after bullet/wolf processing
    {
        bool allDead = true;
        for (const auto& w : worldManager_->getWolves()) { if (w.alive) { allDead = false; break; } }
        if (allDead) {
            std::cout << "YOU WON!!" << std::endl;
            initialized_ = false;
        }
    }

    worldManager_->processMushroomCollisions(carPos); // now no-op
    worldManager_->update(dt, carPos);

    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime(), maxBlur_, blurDecayRate_);

    turret_.update(dt);
    turret_.updateBullets(dt, *scene_);
    updateBears(bears_, dt);
    cameraRig_->follow(*camera_, *car_.getGroup());

    // Wolf collision end game via callback prints lived time only once here if stopped
    if (!initialized_) {
        // Ensure message printed if wolves ended game through callback (CAR vs WOLF)
        // Time printing moved to wolf collision callback? For now we keep original logic when stopping.
    }

    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);
}
