//
// Created by sindr on 11/17/2025.
//

#include "Game.hpp"
#include <iostream>
#include <iomanip>
#include <unordered_set>

using namespace threepp;

Game::Game()
    : canvas_("Mappeoppgave"),
      renderer_(canvas_.size()),
      scene_(threepp::Scene::create()),
      camera_(threepp::PerspectiveCamera::create(75, canvas_.aspect(), 0.1f, 1000)),
      collisionDetection_(7.0f),
      turret_("../Models/turretV1bla4.obj", 3.0f) {
    renderer_.setClearColor(threepp::Color::white);
    scene_->background = threepp::Color::aliceblue;

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

    canvas_.addKeyListener(*cameraRig_); // allow L key to reverse view

    blurOverlay_ = std::make_unique<BlurOverlay>();
    worldManager_ = std::make_unique<WorldManager>(*scene_, collisionDetection_, loader_, blurLevel_);
    worldManager_->spawnWorld({});
    worldManager_->addSkyAndSun(35.0f, 135.0f); // soft morning/evening light

    // Register car with world collision system (wolf hit ends game with survival time)
    worldManager_->registerCar(car_.getGroup(), carCollisionRadius_, [this]{
        if (!initialized_) return;
        initialized_ = false;
        double lived = clock_.getElapsedTime();
        std::cout << "YOU LIVED FOR " << std::fixed << std::setprecision(1) << lived << " seconds" << std::endl;
    });

    controller_ = std::make_unique<KeyController>(*car_.getGroup(), collisionDetection_);
    canvas_.addKeyListener(*controller_);

    canvas_.addKeyListener(turret_);

    // Bears and slowdown are fully handled inside WorldManager; we only provide the effect
    worldManager_->setBearCollisionHandler([this]{
        if (bearCollisionCooldown_ <= 0.0f) {
            controller_->adjustMaxSpeed(-15.0f);
            if (controller_->getMaxSpeed() < 30.0f) controller_->setMaxSpeed(30.0f);
            bearCollisionCooldown_ = 1.0f;
        }
    });
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

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ = std::max(0.0f, bearCollisionCooldown_ - dt);

    controller_->update(dt);

    car_.setSpeed(controller_->getSpeed());
    car_.setRotation(controller_->getRotation());
    car_.update(dt);

    // Let WorldManager sync bullets and process all collisions + wolves
    worldManager_->update(dt, car_.getPosition(), turret_.getBullets());

    // Victory check
    if (worldManager_->allWolvesDead()) {
        std::cout << "YOU WON!!" << std::endl;
        initialized_ = false;
    }

    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime(), maxBlur_, blurDecayRate_);

    turret_.update(dt);
    turret_.updateBullets(dt, *scene_);
    updateBears(bears_, dt);
    cameraRig_->follow(*camera_, *car_.getGroup());
    worldManager_->updateSkyPosition(camera_->position);

    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);
}
