#ifndef BEARS_HPP
#define BEARS_HPP

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>
#include <string>

class Bear {
public:
    Bear(std::shared_ptr<threepp::Object3D> mesh);
    
    void update(float dt);
    std::shared_ptr<threepp::Object3D> getModel() const { return object_; }

private:
    std::shared_ptr<threepp::Object3D> object_;
    float dx_ = 0.0f;
    float dz_ = 0.0f;
    float timer_ = 0.0f;
};

std::vector<Bear> createBears(threepp::Scene& scene, int count, const std::string& modelPath, float scale);

#endif