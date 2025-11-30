//
// Created by sindr on 11/10/2025.
//

#ifndef MAPPEOPPGAVE_TURRET_H
#define MAPPEOPPGAVE_TURRET_H

#include <threepp/threepp.hpp>
#include "bullet.hpp"
#include <memory>
#include <string>
#include <vector>

class Turret : public threepp::KeyListener {
public:
    explicit Turret(const std::string& modelPath, float turnspeedRad = 1.5f);

    void attachTo(threepp::Object3D& parent, const threepp::Vector3& localOffset);
    void setRotation(float x, float y, float z);
    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void update(float dt);
    void setShootDelay(float seconds){shootDelay_ = seconds;};
    std::shared_ptr<threepp::Object3D> object() const;
    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

    const std::vector<std::shared_ptr<Bullet>>& getBullets() const;
    void updateBullets(float dt, threepp::Scene& scene);

private:
    void shoot();

    std::shared_ptr<threepp::Object3D> model_;
    std::vector<std::shared_ptr<Bullet>> bullets_;
    float turnSpeed_;
    float shootCooldown_ = 0.0f;
    float shootDelay_ = 0.5f;
    struct KeyState { bool left = false; bool right = false; bool shoot = false; } keys_;
};

#endif //MAPPEOPPGAVE_TURRET_H