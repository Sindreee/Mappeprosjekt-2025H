//
// Turret class implementation
// Responsible for managing turret model, rotation, shooting bullets, and updating in the scene
//

#include "turret.hpp"
#include <threepp/loaders/AssimpLoader.hpp>
#include <stdexcept>
#include <cmath>

using namespace threepp;

Turret::Turret(const std::string& modelPath, float turnSpeedRad)
    : turnSpeed_(turnSpeedRad) {
    AssimpLoader loader;
    model_ = loader.load(modelPath);
    if (!model_) {
        throw std::runtime_error("Failed to load turret model: " + modelPath);
    }

    // Enable shadows on all meshes that make up the turret.
    model_->traverseType<Mesh>([](Mesh& mesh) {
        mesh.castShadow = true;
        mesh.receiveShadow = true;
    });
}

void Turret::attachTo(Object3D& parent, const Vector3& localOffset) {
    if (!model_) return;
    model_->position.copy(localOffset);
    parent.add(model_);
}

void Turret::setRotation(float x, float y, float z) {
    model_->rotation.set(x, y, z);
}

void Turret::setPosition(float x, float y, float z) {
    model_->position.set(x, y, z);
}

void Turret::setScale(float x, float y, float z) {
    model_->scale.set(x, y, z);
}

void Turret::shoot() {
    if (!model_) return;

    Vector3 worldPos;
    model_->getWorldPosition(worldPos);

    // Combine turret's own yaw with the parent's yaw (e.g. the car) to get a world rotation.
    float worldRotation = model_->rotation.y;
    if (model_->parent) {
        worldRotation += model_->parent->rotation.y;
    }

    float barrelLength = 2.0f;
    float downwardAngle = -0.015f;
    float heightOffset = 3.0f;

    // Forward direction for the bullet in world space (slightly pitched downward).
    Vector3 direction(
        -std::sin(worldRotation) * std::cos(downwardAngle),
        std::sin(downwardAngle),
        -std::cos(worldRotation) * std::cos(downwardAngle)
    );

    Vector3 startPos = worldPos;
    startPos.x += direction.x * barrelLength;
    startPos.y += direction.y * barrelLength + heightOffset;
    startPos.z += direction.z * barrelLength;

    auto bullet = std::make_shared<Bullet>("../Models/bullet/bullet.obj",
        startPos,
        direction,
        worldRotation,
        150.0f);
    bullets_.push_back(bullet);
}

void Turret::update(float dt) {
    if (!model_) return;

    float yawDelta = 0.0f;
    if (keys_.left) yawDelta += turnSpeed_ * dt;
    if (keys_.right) yawDelta -= turnSpeed_ * dt;

    model_->rotation.y += yawDelta;

    shootCooldown_ -= dt;
    if (keys_.shoot && shootCooldown_ <= 0.0f) {
        shoot();
        shootCooldown_ = shootDelay_;
    }
}

void Turret::updateBullets(float dt, Scene& scene) {
    for (auto it = bullets_.begin(); it != bullets_.end();) {
        (*it)->update(dt);

        if (!(*it)->isAlive()) {
            scene.remove(*(*it)->getModel());
            it = bullets_.erase(it);
        } else {
            // Lazily add bullets to the scene the first time they are updated.
            if ((*it)->getModel()->parent == nullptr) {
                scene.add((*it)->getModel());
            }
            ++it;
        }
    }
}

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

const std::vector<std::shared_ptr<Bullet>>& Turret::getBullets() const {
    return bullets_;
}

std::shared_ptr<Object3D> Turret::object() const {
    return model_;
}