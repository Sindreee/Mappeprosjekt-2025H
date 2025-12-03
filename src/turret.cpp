#include "turret.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <stdexcept>
#include <cmath>

using namespace threepp;


Turret::Turret(const std::string& modelPath, float turnSpeedRad)
    : turnSpeed_(turnSpeedRad) {

    // since the turret is one of the critical game components, we throw if loading fails signaling a runtime error
    AssimpLoader loader;
    model_ = loader.load(modelPath);
    if (!model_) {
        throw std::runtime_error("Failed to load turret model: " + modelPath);
    }

    // Enable shadows on turret
    model_->traverseType<Mesh>([](Mesh& mesh) {
        mesh.castShadow = true;
        mesh.receiveShadow = true;
    });
}

// Attach the turret to the Deer at a local offset, originally had setPosition but changed to use localOffset in  attachTo
void Turret::attachTo(Object3D& parent, const Vector3& localOffset) {
    if (!model_) return;
    model_->position.copy(localOffset);
    parent.add(model_);
}

void Turret::setRotation(float x, float y, float z) {
    model_->rotation.set(x, y, z);
}

void Turret::setScale(float x, float y, float z) {
    model_->scale.set(x, y, z);
}


// Shoot a bullet from the turret's current world position and orientation
// Also handles som turret related bullet logic because its simpler to do here than in bullet.cpp
void Turret::shoot() {
    if (!model_) return;

    Vector3 worldPos;
    model_->getWorldPosition(worldPos);

    // Combine the turret yaw with its parent's yaw (deer) to get the final world-space aim
    float worldRotation = model_->rotation.y;
    if (model_->parent) {
        worldRotation += model_->parent->rotation.y;
    }

    float barrelLength = 2.0f;   // how far in front of the pivot the bullet should appear
    float downwardAngle = -0.015f; // small negative pitch so bullets arc slightly down
    float heightOffset = 3.0f;   // raise bullets above the car body to fit inside turret at start

    // Compute a normalized forward direction for the bullet in world space.
    Vector3 direction(
        -std::sin(worldRotation) * std::cos(downwardAngle),
        std::sin(downwardAngle),
        -std::cos(worldRotation) * std::cos(downwardAngle)
    );

    Vector3 startPos = worldPos;
    startPos.x += direction.x * barrelLength;
    startPos.y += direction.y * barrelLength + heightOffset;
    startPos.z += direction.z * barrelLength;

    // Spawn a new bullet starting at the end of the barrel, moving along the given direction
    auto bullet = std::make_shared<Bullet>("Models/bullet/bullet.obj",
        startPos,
        direction,
        worldRotation,
        150.0f);
    bullets_.push_back(bullet);
}


// Rotates turret based on key input and handles shooting with cooldown
void Turret::update(float dt) {
    if (!model_) return;

    float yawDelta = 0.0f;
    if (keys_.left) yawDelta += turnSpeed_ * dt;
    if (keys_.right) yawDelta -= turnSpeed_ * dt;

    model_->rotation.y += yawDelta;

    // Cooldown timer prevents firing more often than shootDelay_.
    shootCooldown_ -= dt;
    if (keys_.shoot && shootCooldown_ <= 0.0f) {
        shoot();
        shootCooldown_ = shootDelay_;
    }
}

// Update all active bullets, removing expired ones from the scene and local list
// and making shure live ones are added to the scene
void Turret::updateBullets(float dt, Scene& scene) {
    for (auto it = bullets_.begin(); it != bullets_.end();) {
        (*it)->update(dt);

        if (!(*it)->isAlive()) {

            scene.remove(*(*it)->getModel());
            it = bullets_.erase(it);
        } else {

            if ((*it)->getModel()->parent == nullptr) {
                scene.add((*it)->getModel());
            }
            ++it;
        }
    }
}


//Key event handlers for rotation control for the turret
void Turret::onKeyPressed(KeyEvent evt) {
    if (evt.key == Key::J) keys_.left = true;
    if (evt.key == Key::K) keys_.right = true;
    if (evt.key == Key::SPACE) keys_.shoot = true;
}

void Turret::onKeyReleased(KeyEvent evt) {
    if (evt.key == Key::J) keys_.left = false;
    if (evt.key == Key::K) keys_.right = false;
    if (evt.key == Key::SPACE) keys_.shoot = false;
}


// Accessor for the list of active bullets
const std::vector<std::shared_ptr<Bullet>>& Turret::getBullets() const {
    return bullets_;
}

// Accessor for the turrets 3D model
std::shared_ptr<Object3D> Turret::object() const {
    return model_;
}