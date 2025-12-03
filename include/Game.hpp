#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
#include <threepp/threepp.hpp>
#include <threepp/canvas/Canvas.hpp>
#include <threepp/loaders/AssimpLoader.hpp>

#include "CameraRig.hpp"
#include "car.hpp"
#include "carKeyController.hpp"
#include "CollisionDetection.hpp"
#include "turret.hpp"
#include "WorldManager.hpp"
#include "BlurOverlay.hpp"
#include "DebugUI.hpp"

// Top-level game controller.
// Owns the rendering canvas, scene, camera, and all major subsystems
// (world manager, player car, input, turret, blur overlay and debug UI)
class Game {
public:
    Game();

    // Enter the main animation loop; returns when the window is closed
    int run();

private:
    friend class DebugUI;

    // Build the initial scene graph and connect world, car, collision and UI
    void initializeScene();

    // Configure and attach the turret to the car model
    void setupTurret();

    // Install window and keyboard event handlers
    void configureEvents();

    // Per-frame update function called from the canvas animation loop
    void tick();

    // Reset game state and respawn dynamic entities
    void restartGame();

    threepp::Canvas canvas_;
    threepp::GLRenderer renderer_;
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera_;

    Car car_;
    CollisionDetection collisionDetection_;
    std::unique_ptr<KeyController> controller_;
    Turret turret_;
    
    std::unique_ptr<threepp::OrbitControls> orbitControls_;
    std::unique_ptr<CameraRig> cameraRig_;
    std::unique_ptr<WorldManager> worldManager_;
    std::unique_ptr<BlurOverlay> blurOverlay_;
    std::unique_ptr<DebugUI> debugUI_;

    float bearCollisionCooldown_ = 0.0f;
    threepp::AssimpLoader loader_;
    threepp::Clock clock_;

    float blurLevel_ = 0.0f;
    bool initialized_ = true;  // When false, tick() stops updating, (victory)
    bool carDestroyed_ = false;
    int difficultyLevel_ = 1;
};

#endif