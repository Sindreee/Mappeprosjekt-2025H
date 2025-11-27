//
// Created by sindr on 11/17/2025.
//

#ifndef MAPPEOPPGAVE_GAME_CPP_H
#define MAPPEOPPGAVE_GAME_CPP_H

#include <memory>
#include <vector>

#include <threepp/threepp.hpp>
#include <threepp/canvas/Canvas.hpp>
#include <threepp/loaders/AssimpLoader.hpp>

#include "CameraRig.hpp"
#include "car.hpp"
#include "carKeyController.hpp"
#include "CollisionDetection.hpp"
#include "turret.hpp"
#include "bears.hpp"
#include "WorldManager.hpp"
#include "BlurOverlay.hpp"
#include "DebugUI.hpp" // Include full definition for std::unique_ptr

class Game {
public:
    Game();
    int run();

private:
    friend class DebugUI; // allow UI panels to access internal state
    void initializeScene();
    void setupTurret();
    void configureEvents();
    void tick();
    void handleWolfHit(); // wolf melee damage handler
    void restartGame(); // extracted restart logic for reuse (UI + key)

    threepp::Canvas canvas_;
    threepp::GLRenderer renderer_;
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera_;

    Car car_;
    CollisionDetection collisionDetection_;
    std::unique_ptr<KeyController> controller_;
    Turret turret_;
    std::vector<MovingBear> bears_;

    std::unique_ptr<threepp::OrbitControls> orbitControls_;
    std::unique_ptr<CameraRig> cameraRig_;
    std::unique_ptr<WorldManager> worldManager_;
    std::unique_ptr<BlurOverlay> blurOverlay_;
    std::unique_ptr<DebugUI> debugUI_;


    // Collision radius
    float carCollisionRadius_ = 7.0f;
    float bearCollisionRadius_ = 7.0f;
    float bearCollisionCooldown_ = 0.0f; // seconds

    threepp::AssimpLoader loader_;
    threepp::Clock clock_;

    float blurLevel_ = 0.0f;
    const float maxBlur_ = 8.0f;
    const float blurDecayRate_ = 0.2f;

    bool initialized_ = true;

    int carHitCount_ = 0;          // wolf melee hits taken
    float carHitCooldown_ = 0.0f;  // cooldown between registering hits
    const int carHitsToDie_ = 10;   // hits needed to die
    bool carDestroyed_ = false;    // car death state
    
    int difficultyLevel_ = 1;      // Difficulty level (1, 2, or 3) - affects wolf speed
};

#endif //MAPPEOPPGAVE_GAME_CPP_H
