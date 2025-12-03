#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <threepp/threepp.hpp>
#include <functional>
#include <memory>

// Could have removed this class but chose to keep it for potential use with more idle game objects like stones

class GameObject {
public:
    using CollisionCallback = std::function<void(GameObject&)>;

    GameObject(std::shared_ptr<threepp::Object3D> mesh);

    // Register a callback that is called when the collision system reports a hit
    void setOnCollision(CollisionCallback callback);

    // Invoke the registered callback
    void handleCollision();

    // Remove this object's model from the given scene, if it is currently attached
    void removeFromScene(threepp::Scene& scene);

    std::shared_ptr<threepp::Object3D> getModel() const { return model_; }

private:
    std::shared_ptr<threepp::Object3D> model_;
    CollisionCallback onCollision_;
};

#endif