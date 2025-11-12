#ifndef MAPPEOPPGAVE_CARKEYCONTROLLER_HPP
#define MAPPEOPPGAVE_CARKEYCONTROLLER_HPP

#include "threepp/threepp.hpp"
#include "CollisionDetection.hpp"

class KeyController : public threepp::KeyListener {
public:
    KeyController(threepp::Object3D& object, CollisionDetection& collisionDetection,
                  float acceleration = 50.0f, float maxSpeed = 150.0f,
                  float friction = 0.9f, float turnSpeed = 2.0f);

    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;
    void update(float dt);

private:
    threepp::Object3D* object_;
    CollisionDetection* collisionDetection_;
    float acceleration_;
    float maxSpeed_;
    float friction_;
    float turnSpeed_;
    float velocity_ = 0.0f;

    struct KeyState {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keyState_;
};

#endif
