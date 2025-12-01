#include "Game.hpp"
#include "GameConstants.hpp"
#include "DebugUI.hpp"
#include <iostream>

struct GLFWwindow;

using namespace threepp;

Game::Game()
    : canvas_("Bilsimulator 2026"),
      renderer_(canvas_.size()),
      scene_(Scene::create()),
      camera_(PerspectiveCamera::create(75, canvas_.aspect(), 0.1f, 1000)),
      collisionDetection_(Config::CAR_COLLISION_RADIUS),
      turret_("../Models/turretV1bla4.obj", 3.0f)
{
    renderer_.setClearColor(Color::white);
    renderer_.shadowMap().enabled = true;
    scene_->background = Color::aliceblue;

    initializeScene();
    setupTurret();
    configureEvents();

    debugUI_ = std::make_unique<DebugUI>(*this, static_cast<GLFWwindow*>(canvas_.windowPtr()));
}

int Game::run() {
    canvas_.animate([&]() { tick(); });
    return 0;
}

void Game::initializeScene() {
    scene_->add(car_.getGroup());

    cameraRig_ = std::make_unique<CameraRig>();
    orbitControls_ = std::make_unique<OrbitControls>(*camera_, canvas_);
    orbitControls_->target.set(0, 0, 0);
    orbitControls_->update();
    canvas_.addKeyListener(*cameraRig_);

    blurOverlay_ = std::make_unique<BlurOverlay>();
    worldManager_ = std::make_unique<WorldManager>(*scene_, collisionDetection_, loader_, blurLevel_);

    worldManager_->spawnWorld({});

    // Original static sun setup
    worldManager_->addSkyAndSun(35.0f, 135.0f);

    // --- CALLBACKS ---
    worldManager_->registerCar(car_.getGroup(), Config::CAR_COLLISION_RADIUS, [this]{
        car_.takeDamage(1);
    });

    worldManager_->setMushroomCollisionHandler([this]{
        // Only handles visual/audio cue here. Speed boost logic is in tick()
    });

    worldManager_->setBearCollisionHandler([this]{
        if (bearCollisionCooldown_ <= 0.0f) {
            car_.modifyBaseSpeed(-15.0f); // Permanent penalty (until restart)
            std::cout << "Hit bear! Speed reduced." << std::endl;
            bearCollisionCooldown_ = 1.0f;
        }
    });

    controller_ = std::make_unique<KeyController>(*car_.getGroup(), collisionDetection_);
    canvas_.addKeyListener(*controller_);
    canvas_.addKeyListener(turret_);
}

void Game::setupTurret() {
    turret_.setRotation(0, 0, 0);
    turret_.setPosition(0, -100.0f, 0);
    turret_.setScale(0.03f, 0.03f, 0.03f);
    turret_.attachTo(*car_.getGroup(), Vector3(0, 1.0f, 0));
}

void Game::configureEvents() {
    canvas_.onWindowResize([&](WindowSize size) {
        camera_->aspect = size.aspect();
        camera_->updateProjectionMatrix();
        renderer_.setSize(size);
    });

    struct RestartListener : public KeyListener {
        Game& g;
        RestartListener(Game& game) : g(game) {}
        void onKeyPressed(KeyEvent evt) override {
            if (evt.key == Key::R) g.restartGame();
        }
    };
    static auto listener = std::make_shared<RestartListener>(*this);
    canvas_.addKeyListener(*listener);
}

void Game::restartGame() {
    initialized_ = true;
    carDestroyed_ = false;
    blurLevel_ = 0.0f;
    bearCollisionCooldown_ = 0.0f;

    car_.reset();
    worldManager_->setDifficultyLevel(difficultyLevel_);
    worldManager_->resetDynamicEntities();

    clock_.start();
    std::cout << "Game restarted! Difficulty: " << difficultyLevel_ << std::endl;
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ -= dt;

    // --- LOGIC FIX: Decay speed boost with blur ---
    // This prevents infinite speed accumulation. As blur fades, speed returns to normal.
    float currentBoost = Config::MUSHROOM_SPEED_BOOST * blurLevel_;
    car_.applySpeedBoost(currentBoost);

    // 1. INPUT
    if (!car_.isDead() && !car_.isFalling()) {
        controller_->setMaxSpeed(car_.getMaxSpeed());
        controller_->update(dt);

        car_.setSpeed(controller_->getSpeed());
        car_.setRotation(controller_->getRotation());
    } else {
        car_.setSpeed(0.0f);
    }

    // 2. BIL
    car_.update(dt);

    // 3. GAME OVER
    if (car_.isDead() && !carDestroyed_) {
        std::cout << "GAME OVER! The deer died." << std::endl;
        carDestroyed_ = true;
    }

    // 4. VERDEN
    worldManager_->update(dt, car_.getPosition(), turret_.getBullets());

    if (worldManager_->allWolvesDead()) {
        std::cout << "VICTORY! All wolves eliminated." << std::endl;
        initialized_ = false;
    }

    // 5. EFFEKTER
    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime(), Config::MAX_BLUR, 0.2f);

    turret_.update(dt);
    turret_.updateBullets(dt, *scene_);

    cameraRig_->follow(*camera_, *car_.getGroup());
    worldManager_->updateSkyPosition(camera_->position);

    // 6. RENDER
    if (debugUI_) debugUI_->newFrame();
    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);
    if (debugUI_) debugUI_->render();
}