#ifndef BEARS_HPP
#define BEARS_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>
#include <string>

// Bear randomly wandering around the plane
class Bear {
public:
    explicit Bear(std::shared_ptr<threepp::Object3D> mesh);

    // Update the bear's movement
    void update(float dt);

    // Access the underlying Threepp object so callers can attach it to the scene or colliders
    std::shared_ptr<threepp::Object3D> getModel() const { return object_; }


private:
    std::shared_ptr<threepp::Object3D> object_;
    float dx_ = 0.0f;    // X direction
    float dz_ = 0.0f;    // Z direction
    float timer_ = 0.0f; // Time accumulator used to pick new directions
};

// Load a bear model and spawn multiple Bear instances in random positions.
std::vector<Bear> createBears(threepp::Scene& scene, int count, const std::string& modelPath, float scale);

#endif