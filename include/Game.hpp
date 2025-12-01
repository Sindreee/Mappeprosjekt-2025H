#ifndef GAME_HPP
#define GAME_HPP

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
#include "Bear.hpp"
#include "WorldManager.hpp"
#include "BlurOverlay.hpp"
#include "DebugUI.hpp"

class Game {
public:
    Game();
    int run();

private:
    friend class DebugUI;
    void initializeScene();
    void setupTurret();
    void configureEvents();
    void tick();
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
    bool initialized_ = true;
    bool carDestroyed_ = false;
    int difficultyLevel_ = 1;
};

#endif