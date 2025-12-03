#ifndef MAPPEOPPGAVE_WOLF_HPP
#define MAPPEOPPGAVE_WOLF_HPP
#include <threepp/threepp.hpp>
#include <memory>
#include <string>
#include <vector>

// Forward declaration so we can use threepp::Scene in the function signature without including full threepp headers.
namespace threepp { class Scene; }


class Wolf {
public:
    Wolf(std::shared_ptr<threepp::Object3D> mesh,
         float maxSpeed,
         float acceleration,
         threepp::Vector3 offset);


    void update(float dt, const threepp::Vector3& targetPos);

    // Apply one hit of damage; wolves speed up when hurt and die after enough hits
    void takeDamage();

    bool isAlive() const { return alive_; }
    int getHits() const { return hits_; }
    std::shared_ptr<threepp::Object3D> getModel() const { return object_; }

    void setMaxSpeed(float speed) { maxSpeed_ = speed; }

private:
    std::shared_ptr<threepp::Object3D> object_;
    threepp::Vector3 velocity_{0, 0, 0};
    float maxSpeed_;
    float acceleration_;
    threepp::Vector3 targetOffset_;

    float noiseOffset_;
    float timeAlive_ = 0.0f;

    int hits_ = 0;
    bool alive_ = true;
    const int hitsToDie_ = 5;
};

// Load a wolf model and spawn a number of wolves with stats from the difficulty factor.
std::vector<Wolf> createWolves(threepp::Scene& scene, int count, const std::string& modelPath, float difficultyFactor);

#endif //MAPPEOPPGAVE_WOLF_HPP