#include "BlurOverlay.hpp"
#include "GameConstants.hpp"
#include <algorithm>
#include <cmath>

// Creates a tiny overlay PlaneGoem that covers the camera and is rendered to visualize blur level
// Did use AI to help with Opacity and OrthographicCamera setup and edited the functionality and parameters to fit the program needs
BlurOverlay::BlurOverlay() {
    orthoCamera_ = threepp::OrthographicCamera::create(-1, 1, 1, -1, 0, 1);
    blurScene_ = threepp::Scene::create();

    // Fullscreen quad rendered after the main scene to visualize the blur level
    auto blurGeometry = threepp::PlaneGeometry::create(2, 2);
    blurMaterial_ = threepp::MeshBasicMaterial::create();
    blurMaterial_->color = threepp::Color(0.80f, 0.20f, 0.4f);
    blurMaterial_->transparent = true;
    blurMaterial_->opacity = 0.0f;
    blurMaterial_->depthTest = false;

    auto blurPlane = threepp::Mesh::create(blurGeometry, blurMaterial_);
    blurScene_->add(blurPlane);
}


void BlurOverlay::update(float& blurLevel, float dt, float elapsedTime) {
    const float decayRate = 0.2f;

    // Blur level slowly decays over time to simulate the effect wearing off
    blurLevel = std::max(0.0f, blurLevel - decayRate * dt);

    if (blurLevel <= 0.0f) {
        blurMaterial_->opacity = 0.0f;
        return;
    }

    // Add leveling to blur for increased game variation, varying intensity and pulsing for better game mushroom punishement
    float intensity = blurLevel / Config::MAX_BLUR;

    if (blurLevel <= 2.0f) {

        blurMaterial_->opacity = intensity * 0.3f;
    } else if (blurLevel <= 5.0f) {

        blurMaterial_->opacity = 0.3f + (intensity - 0.25f) * 0.4f;
    } else {

        blurMaterial_->opacity = 0.5f + (intensity - 0.625f) * 1.2f;
        float pulse = 0.1f * std::sin(elapsedTime * 8.0f);
        blurMaterial_->opacity = std::min(0.95f, blurMaterial_->opacity + pulse);
    }
}

// Render the blur quad on top of whatever the main renderer has already drawn
void BlurOverlay::render(threepp::GLRenderer& renderer) {
    renderer.autoClear = false;
    renderer.render(*blurScene_, *orthoCamera_);
    renderer.autoClear = true;
}