#ifndef MAPPEOPPGAVE_TURRET_H
#define MAPPEOPPGAVE_TURRET_H

#include <threepp/threepp.hpp>
#include "bullet.hpp"
#include <memory>
#include <string>
#include <vector>

// Player-controlled turret mounted on the car
// Rotates left/right from keyboard input and periodically fires Bullets

class Turret : public threepp::KeyListener {
public:
    explicit Turret(const std::string& modelPath, float turnspeedRad = 1.5f);

    // Attach the turret model to a parent object at the given local offset.
    void attachTo(threepp::Object3D& parent, const threepp::Vector3& localOffset);

    void setRotation(float x, float y, float z);
    void setScale(float x, float y, float z);

    // Update turret rotation and internal shoot cooldown timer.
    void update(float dt);

    std::shared_ptr<threepp::Object3D> object() const;

    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

    const std::vector<std::shared_ptr<Bullet>>& getBullets() const;

    // Update all bullets, remove expired ones, and add live bullets to the scene on first use
    void updateBullets(float dt, threepp::Scene& scene);

private:
    void shoot();

    std::shared_ptr<threepp::Object3D> model_;
    std::vector<std::shared_ptr<Bullet>> bullets_;
    float turnSpeed_;
    float shootCooldown_ = 0.0f;
    float shootDelay_ = 0.4f;
    struct KeyState { bool left = false; bool right = false; bool shoot = false; } keys_;
};

#endif //MAPPEOPPGAVE_TURRET_H