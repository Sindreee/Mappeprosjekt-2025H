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
    worldManager_->addSkyAndSun(35.0f, 135.0f);

    // Register car ONLY ONCE with melee callback (wolf collision damages car)
    worldManager_->registerCar(car_.getGroup(), carCollisionRadius_, [this]{ handleWolfHit(); });

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

    class RestartListener : public threepp::KeyListener {
    public:
        RestartListener(Game& game): game_(game) {}
        void onKeyPressed(threepp::KeyEvent evt) override {
            if (evt.key == threepp::Key::R) {
                game_.initialized_ = true;
                game_.bearCollisionCooldown_ = 0.0f;
                game_.blurLevel_ = 0.0f;
                game_.carHitCount_ = 0;
                game_.carHitCooldown_ = 0.0f;
                game_.carDestroyed_ = false;
                if (game_.controller_) game_.controller_->setMaxSpeed(150.0f);
                game_.car_.reset();
                game_.worldManager_->resetDynamicEntities();
                game_.clock_.start();
                std::cout << "Game reset" << std::endl;
            }
        }
        void onKeyReleased(threepp::KeyEvent) override {}
    private:
        Game& game_;
    };
    static auto restartListener = std::make_unique<RestartListener>(*this);
    canvas_.addKeyListener(*restartListener);
}

void Game::handleWolfHit() {
    if (carDestroyed_) return;
    if (carHitCooldown_ > 0.0f) return; // prevent rapid multi-count

    carHitCount_++;
    carHitCooldown_ = 0.5f; // half second cooldown
    std::cout << "Car hit by wolf (" << carHitCount_ << "/" << carHitsToDie_ << ")" << std::endl;

    // Optional feedback: reduce max speed slightly each hit
    if (controller_) controller_->adjustMaxSpeed(-5.0f);

    if (carHitCount_ >= carHitsToDie_) {
        carDestroyed_ = true;
        initialized_ = false; // stop game loop
        std::cout << "CAR DESTROYED. YOU LOSE." << std::endl;
    }
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ = std::max(0.0f, bearCollisionCooldown_ - dt);
    if (carHitCooldown_ > 0.0f) carHitCooldown_ = std::max(0.0f, carHitCooldown_ - dt); // fixed condition

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
