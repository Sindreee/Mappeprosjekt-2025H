//
// Created by sindr on 11/17/2025.
//

#include "Game.hpp"
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include "DebugUI.hpp"

struct GLFWwindow; // Forward declaration

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

    // Initialize ImGui with the window handle from canvas
    debugUI_ = std::make_unique<DebugUI>(*this, static_cast<GLFWwindow*>(canvas_.windowPtr()));
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
                game_.restartGame();
            }
        }
        void onKeyReleased(threepp::KeyEvent) override {}
    private:
        Game& game_;
    };
    static auto restartListener = std::make_unique<RestartListener>(*this);
    canvas_.addKeyListener(*restartListener);
}

void Game::restartGame() {
    initialized_ = true;
    bearCollisionCooldown_ = 0.0f;
    blurLevel_ = 0.0f;
    carHitCount_ = 0;
    carHitCooldown_ = 0.0f;
    carDestroyed_ = false;
    if (controller_) controller_->setMaxSpeed(150.0f);
    car_.reset();

    // Apply difficulty level before respawning wolves
    worldManager_->setDifficultyLevel(difficultyLevel_);
    worldManager_->resetDynamicEntities();

    clock_.start();
    std::cout << "Game reset (Difficulty Level " << difficultyLevel_ << ")" << std::endl;
}

void Game::handleWolfHit() {
    if (carDestroyed_) return;
    if (carHitCooldown_ > 0.0f) return; // prevent rapid multi-count

    carHitCount_++;
    carHitCooldown_ = 0.5f; // half second cooldown
    std::cout << "Deer hit by wolf (" << carHitCount_ << "/" << carHitsToDie_ << ")" << std::endl;

    // Optional feedback: reduce max speed slightly each hit
    if (controller_) controller_->adjustMaxSpeed(-5.0f);

    if (carHitCount_ >= carHitsToDie_) {
        carDestroyed_ = true;
        initialized_ = false; // stop game loop
        std::cout << "Deer Eaten Alive. YOU LOSE." << std::endl;
    }
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ = std::max(0.0f, bearCollisionCooldown_ - dt);
    if (carHitCooldown_ > 0.0f) carHitCooldown_ = std::max(0.0f, carHitCooldown_ - dt);

    controller_->update(dt);

    car_.setSpeed(controller_->getSpeed());
    car_.setRotation(controller_->getRotation());
    car_.update(dt);

    worldManager_->update(dt, car_.getPosition(), turret_.getBullets());

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

    // ImGui frame start
    if (debugUI_) debugUI_->newFrame();

    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);

    if (debugUI_) debugUI_->render();
}