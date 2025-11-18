//
// Created by sindr on 11/17/2025.
//

#include "Game.hpp"
#include <iostream>

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

    cameraRig_ = std::make_unique<CameraRig>();
    orbitControls_ = std::make_unique<threepp::OrbitControls>(*camera_, canvas_);
    orbitControls_->target.set(0, 0, 0);
    orbitControls_->update();

    blurOverlay_ = std::make_unique<BlurOverlay>();
    worldManager_ = std::make_unique<WorldManager>(*scene_, collisionDetection_, loader_, blurLevel_);
    worldManager_->spawnWorld({});

    controller_ = std::make_unique<KeyController>(*car_.getGroup(), collisionDetection_);
    canvas_.addKeyListener(*controller_);

    canvas_.addKeyListener(turret_);

    try {
        spawnBears(*scene_, bears_, 500, "../Models/Mesh_Bear.obj", 0.1f);
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
    const float collideDist = carCollisionRadius_ + bearCollisionRadius_;
    const float collideDistSq = collideDist * collideDist;

    for (const auto& b : bears_) {
        const auto& p = b.object->position;
        const float dx = p.x - carPos.x;
        const float dz = p.z - carPos.z;
        const float d2 = dx * dx + dz * dz;
        if (d2 <= collideDistSq) {
            if (bearCollisionCooldown_ <= 0.0f) {
                controller_->adjustMaxSpeed(-15.0f);
                if (controller_->getMaxSpeed() < 30.0f) {
                    controller_->setMaxSpeed(30.0f);
                }
                bearCollisionCooldown_ = 1.0f; // seconds between penalties
            }
            return;
        }
    }
}

void Game::processWolfCollisions(const threepp::Vector3& carPos) {
    if (!worldManager_) return;
    const float collideDist = carCollisionRadius_ + wolfCollisionRadius_;
    const float collideDistSq = collideDist * collideDist;

    const auto& wolves = worldManager_->getWolves();
    for (const auto& w : wolves) {
        const auto& p = w.object->position;
        const float dx = p.x - carPos.x;
        const float dz = p.z - carPos.z;
        const float d2 = dx * dx + dz * dz;
        if (d2 <= collideDistSq) {
            std::cerr << "Game over: Deer collided with a wolf." << std::endl;
            initialized_ = false; // keep window open, stop updating
            return;
        }
    }
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
    processBearCollisions(carPos);
    processWolfCollisions(carPos);
    worldManager_->processMushroomCollisions(carPos);
    worldManager_->update(dt, carPos);

    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime(), maxBlur_, blurDecayRate_);

    turret_.update(dt);
    turret_.updateBullets(dt, *scene_);
    updateBears(bears_, dt);
    cameraRig_->follow(*camera_, *car_.getGroup());

    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);
}
