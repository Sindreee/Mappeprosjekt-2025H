//
// Created by sindr on 11/17/2025.
//

#ifndef MAPPEOPPGAVE_BULLET_HPP
#define MAPPEOPPGAVE_BULLET_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <string>

class Bullet {
public:
    explicit Bullet(const std::string& modelPath,
               const threepp::Vector3& startPos,
               const threepp::Vector3& direction,
               float rotation,
               float speed = 50.0f,
               float lifetime = 5.0f);

    void update(float dt);
    bool isAlive() const;
    std::shared_ptr<threepp::Object3D> getModel() const;
    threepp::Vector3 getPosition() const;

private:
    std::shared_ptr<threepp::Object3D> model_;
    threepp::Vector3 velocity_;
    float lifetime_;
    float elapsed_ = 0.0f;
};


#endif //MAPPEOPPGAVE_BULLET_HPP