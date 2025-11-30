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
    // Enable shadow mapping so lights like the sun can cast shadows
    renderer_.shadowMap().enabled = true; // use default shadow type; PCFShadow not available in this version
    scene_->background = threepp::Color::aliceblue;

    initializeScene();
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
        // Permanent slowdown: reduce the base max speed used in tick()
        baseMaxSpeed_ = std::max(30.0f, baseMaxSpeed_ - 15.0f);
        bearCollisionCooldown_ = 1.0f;
    }
});

    // Mushroom pickup handler - blur effect is already handled internally, just notify for future sound
    worldManager_->setMushroomCollisionHandler([this]{
        // Blur level is already increased inside WorldManager
        // No permanent speed change here; speed boost is derived from blurLevel_ in tick()
    });
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

    baseMaxSpeed_ = 150.0f;                // reset base to default
    if (controller_) controller_->setMaxSpeed(baseMaxSpeed_);

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



    if (carHitCount_ >= carHitsToDie_) {
        carDestroyed_ = true;
        initialized_ = false; // stop game loop
        std::cout << "Deer Eaten Alive. YOU LOSE." << std::endl;
    }
}

void Game::handleOutOfBoundsAndGravity(float dt) {
    auto carPos = car_.getPosition();

    // If the deer/car is outside the allowed XZ range, treat as out-of-bounds
    bool outOfBounds = (carPos.x < -1500.f || carPos.x > 1500.f ||
                        carPos.z < -1500.f || carPos.z > 1500.f);

    // Static fall velocity so it persists across frames while falling
    static float fallVelocityY = 0.0f;

    if (outOfBounds) {
        // Block any *new* acceleration from input by clamping controller max speed.
        // We do NOT zero the car's existing velocity/rotation here so it can
        // continue with its pre-existing horizontal motion while falling.
        if (controller_) {
            controller_->setMaxSpeed(0.0f);
        }

        // Increment carHitCount_ using carHitCooldown_ as delay
        if (carHitCooldown_ <= 0.0f && !carDestroyed_) {
            carHitCount_++;
            carHitCooldown_ = 0.5f; // reuse the same cooldown as wolf hits
            std::cout << "Deer out of bounds hit (" << carHitCount_ << "/" << carHitsToDie_ << ")" << std::endl;

            if (carHitCount_ >= carHitsToDie_) {
                carDestroyed_ = true;
                initialized_ = false;
                std::cout << "Deer fell out of the world. YOU LOSE." << std::endl;
            }
        }

        // Apply simple gravity while "falling": accelerate straight down in Y
        const float g = 9.81f;
        fallVelocityY -= g * dt;        // accelerate downwards (negative Y)
        carPos.y += fallVelocityY * dt; // integrate position

        // Keep the current X/Z from the car's last motion, only move in Y here
        car_.getGroup()->position.copy(carPos);
    } else {
        // Reset fall velocity when back in bounds (e.g., after restart)
        fallVelocityY = 0.0f;
    }
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ = std::max(0.0f, bearCollisionCooldown_ - dt);
    if (carHitCooldown_ > 0.0f) carHitCooldown_ = std::max(0.0f, carHitCooldown_ - dt);

    // Determine if car is currently out of bounds before applying input
    auto carPos = car_.getPosition();
    bool outOfBounds = (carPos.x < -1500.f || carPos.x > 1500.f ||
                        carPos.z < -1500.f || carPos.z > 1500.f);

    // Only update controller and apply its movement while in-bounds
    if (!outOfBounds) {
        controller_->update(dt);
        car_.setSpeed(controller_->getSpeed());
        car_.setRotation(controller_->getRotation());
        car_.update(dt);
    }

    // Handle leaving world bounds + falling behavior in a dedicated helper
    handleOutOfBoundsAndGravity(dt);

    worldManager_->update(dt, car_.getPosition(), turret_.getBullets());

    if (worldManager_->allWolvesDead()) {
        std::cout << "YOU WON!!" << std::endl;
        initialized_ = false;
    }

    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime(), maxBlur_, blurDecayRate_);

    // Derive mushroom speed bonus from current blur level so speed boost lasts as long as blur
    float mushroomBonus = blurLevel_ * mushroomSpeedFactor_;
    float newMaxSpeed = baseMaxSpeed_ + mushroomBonus;
    // Only adjust controller max speed while in-bounds; once out, it stays clamped to 0 in handleOutOfBoundsAndGravity
    if (controller_ && !outOfBounds) controller_->setMaxSpeed(newMaxSpeed);

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