#include "Game.hpp"
#include "GameConstants.hpp"
#include "DebugUI.hpp"

using namespace threepp;

Game::Game()
    : canvas_("Automation Deer 2026"),
      renderer_(canvas_.size()),
      scene_(Scene::create()),
      camera_(PerspectiveCamera::create(75, canvas_.aspect(), 0.1f, 1000)),
      collisionDetection_(),
      turret_("Models/turretV1bla4.obj", 3.0f)
{
    // Basic renderer setup: white clear color, enabled shadow mapping, and aliceblue background
    renderer_.setClearColor(Color::white);
    renderer_.shadowMap().enabled = true;
    scene_->background = Color::aliceblue;

    // Build initial scene graph, attach game objects, and register collision handlers
    initializeScene();

    setupTurret();
    // Wire up window resize and restart key events.
    configureEvents();

    // Create the ImGui-based debug UI overlay if a valid window is available
    debugUI_ = std::make_unique<DebugUI>(*this, static_cast<GLFWwindow*>(canvas_.windowPtr()));
}

// Start the main loop and call tick() once per rendered frame
int Game::run() {
    canvas_.animate([&]() { tick(); });
    return 0;
}

void Game::initializeScene() {
    // Attach the car to the scene so other systems can follow and collide with it
    scene_->add(car_.getGroup());

    cameraRig_ = std::make_unique<CameraRig>();
    orbitControls_ = std::make_unique<OrbitControls>(*camera_, canvas_);
    orbitControls_->target.set(0, 0, 0);
    orbitControls_->update();
    canvas_.addKeyListener(*cameraRig_);

    blurOverlay_ = std::make_unique<BlurOverlay>();

    // WorldManager owns environment entities and integrates with collision + blur level
    worldManager_ = std::make_unique<WorldManager>(*scene_, collisionDetection_, loader_, blurLevel_);
    worldManager_->spawnWorld({});

    // Adjust eleveationDeg for the Maps Shadow coverage making it harder or easier
    worldManager_->addSkyAndSun(35.0f, 135.0f);

    // Register the car for collisions, wolves will call takeDamage through this callback
    worldManager_->registerCar(car_.getGroup(), Config::CAR_COLLISION_RADIUS, [this]{
        car_.takeDamage(1);
    });

    // Expose mushroom hits to the Game if extra behavior is ever needed
    worldManager_->setMushroomCollisionHandler([this]{});

    // On bear collision, temporarily slow down the car via its base speed
    worldManager_->setBearCollisionHandler([this]{
        if (bearCollisionCooldown_ <= 0.0f) {
            car_.modifyBaseSpeed(-15.0f);
            bearCollisionCooldown_ = 1.0f;
        }
    });

    // Keyboard controller uses collisionDetection_ to drive the car without passing through obstacles
    controller_ = std::make_unique<KeyController>(*car_.getGroup(), collisionDetection_);
    canvas_.addKeyListener(*controller_);
    canvas_.addKeyListener(turret_);
}

void Game::setupTurret() {
    turret_.setRotation(0, 0, 0);
    turret_.setScale(0.03f, 0.03f, 0.03f);
    turret_.attachTo(*car_.getGroup(), Vector3(0, 1.0f, 0));
}

void Game::configureEvents() {
    // Keep camera projection and renderer size in sync with window resizes
    canvas_.onWindowResize([&](WindowSize size) {
        camera_->aspect = size.aspect();
        camera_->updateProjectionMatrix();
        renderer_.setSize(size);
    });

    // Simple key listener used to restart the game when R is pressed
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
    // Reactivate the game loop and clear transient state
    initialized_ = true;
    carDestroyed_ = false;
    blurLevel_ = 0.0f;
    bearCollisionCooldown_ = 0.0f;

    car_.reset();
    worldManager_->setDifficultyLevel(difficultyLevel_);
    worldManager_->resetDynamicEntities();

    // Restart the frame timer so dt is well-defined after a reset
    clock_.start();
}

void Game::tick() {
    if (!initialized_) return;

    float dt = clock_.getDelta();
    if (bearCollisionCooldown_ > 0.0f) bearCollisionCooldown_ -= dt;

    // Drive car speed boost from the current blur level.
    float currentBoost = Config::MUSHROOM_SPEED_BOOST * blurLevel_;
    car_.applySpeedBoost(currentBoost);

    // Only allow direct control while the car is alive and not falling
    if (!car_.isDead() && !car_.isFalling()) {
        controller_->setMaxSpeed(car_.getMaxSpeed());
        controller_->update(dt);

        car_.setSpeed(controller_->getSpeed());
        car_.setRotation(controller_->getRotation());

    } else {
        car_.setSpeed(0.0f);
    }

    car_.update(dt);

    // Latch the "car destroyed" flag as soon as the car dies
    if (car_.isDead() && !carDestroyed_) {
        carDestroyed_ = true;
    }

    // Use the car's world-space position as reference for enemys and collision
    worldManager_->update(dt, car_.getGroup()->position, turret_.getBullets());

    // Stop updating once all wolves are dead
    if (worldManager_->allWolvesDead()) {
        initialized_ = false;
    }

    // Update the visual blur overlay based on the current blur level
    blurOverlay_->update(blurLevel_, dt, clock_.getElapsedTime());

    turret_.update(dt);
    turret_.updateBullets(dt, *scene_);

    cameraRig_->follow(*camera_, *car_.getGroup());

    worldManager_->updateSkyPosition(camera_->position);

    if (debugUI_) debugUI_->newFrame();
    renderer_.render(*scene_, *camera_);
    blurOverlay_->render(renderer_);
    if (debugUI_) debugUI_->render();
}