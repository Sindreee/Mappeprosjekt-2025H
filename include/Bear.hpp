#ifndef BEARS_HPP
#define BEARS_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>
#include <string>

// Simple wandering bear enemy that moves in a changing random direction.

class Bear {
public:
    Bear(std::shared_ptr<threepp::Object3D> mesh);
    
    // Update the bear's wander direction and move it forward. For every dt
    void update(float dt);


    //This returns the underlying Object 3D so the scene can add/remove it. Other systems can access its position and attach colliders.
    std::shared_ptr<threepp::Object3D> getModel() const { return object_; }

private:
    std::shared_ptr<threepp::Object3D> object_;
    float dx_ = 0.0f;   // Current wander direction on X
    float dz_ = 0.0f;   // Current wander direction on Z
    float timer_ = 0.0f; // Time accumulator used to pick new directions
};

// Helper for loading a bear model once and spawning many instances into the scene.
std::vector<Bear> createBears(threepp::Scene& scene, int count, const std::string& modelPath, float scale);

#endif