#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <threepp/threepp.hpp>
#include <functional>
#include <memory>
#include <string>

// Thin wrapper around a Threepp Object3D that can participate in collisions
// via a callback, and can be easily removed from the scene.
class GameObject {
public:
    using CollisionCallback = std::function<void(GameObject&)>;

    GameObject(std::shared_ptr<threepp::Object3D> mesh);

    // Register a callback that is invoked when the collision system reports a hit.
    void setOnCollision(CollisionCallback callback);

    // Invoke the registered collision callback, if any.
    void handleCollision();

    // Remove this object's model from the given scene, if it is currently attached.
    void removeFromScene(threepp::Scene& scene);

    std::shared_ptr<threepp::Object3D> getModel() const { return model_; }

private:
    std::shared_ptr<threepp::Object3D> model_;
    CollisionCallback onCollision_;
};

#endif // GAMEOBJECT_HPP