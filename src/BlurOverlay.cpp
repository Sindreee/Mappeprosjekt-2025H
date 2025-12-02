#include "BlurOverlay.hpp"
#include "GameConstants.hpp"
#include <algorithm>
#include <cmath>

BlurOverlay::BlurOverlay() {
    orthoCamera_ = threepp::OrthographicCamera::create(-1, 1, 1, -1, 0, 1);
    blurScene_ = threepp::Scene::create();

    // Fullscreen quad drawn on top of the main scene to visualize blur/intoxication.
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

    // Blur level slowly decays over time to simulate the effect wearing off.
    blurLevel = std::max(0.0f, blurLevel - decayRate * dt);

    if (blurLevel <= 0.0f) {
        blurMaterial_->opacity = 0.0f;
        return;
    }

    // Convert logical blur level into a visual opacity, with stronger effects at high levels.
    float intensity = blurLevel / Config::MAX_BLUR;

    if (blurLevel <= 2.0f) {
        // Very mild blur when only slightly affected.
        blurMaterial_->opacity = intensity * 0.3f;
    } else if (blurLevel <= 5.0f) {
        // Medium strength in the middle range.
        blurMaterial_->opacity = 0.3f + (intensity - 0.25f) * 0.4f;
    } else {
        // Strong blur at high intoxication levels, with a pulsing effect.
        blurMaterial_->opacity = 0.5f + (intensity - 0.625f) * 1.2f;
        float pulse = 0.1f * std::sin(elapsedTime * 8.0f);
        blurMaterial_->opacity = std::min(0.95f, blurMaterial_->opacity + pulse);
    }
}

void BlurOverlay::render(threepp::GLRenderer& renderer) {
    // Render the blur quad on top of whatever the main renderer has already drawn.
    renderer.autoClear = false;
    renderer.render(*blurScene_, *orthoCamera_);
    renderer.autoClear = true;
}