#ifndef MAPPEOPPGAVE_BULLET_HPP
#define MAPPEOPPGAVE_BULLET_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <string>


// Simple projectile fired by the turret, moves in a straight line and expires after a lifetime
class Bullet {
public:

    // modelPath and other parameters to setup the bullet
    explicit Bullet(const std::string& modelPath,
               const threepp::Vector3& startPos,
               const threepp::Vector3& direction,
               float rotation,
               float speed = 90.0f,
               float lifetime = 5.0f);

    // Expiration accumulation and position update
    void update(float dt);

    // checks if it is alive
    bool isAlive() const;

    // get the threepp object for the bullet
    std::shared_ptr<threepp::Object3D> getModel() const;

    // Gets the current position of the bullet
    threepp::Vector3 getPosition() const;

    // Private members for model, velocity, lifetime, and elapsed time
private:
    std::shared_ptr<threepp::Object3D> model_;
    threepp::Vector3 velocity_;
    float lifetime_;
    float elapsed_ = 0.0f;
};

#endif //MAPPEOPPGAVE_BULLET_HPP