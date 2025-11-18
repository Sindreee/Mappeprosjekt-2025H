#include "BlurOverlay.hpp"
#include <algorithm>
#include <cmath>

BlurOverlay::BlurOverlay() {
    orthoCamera_ = threepp::OrthographicCamera::create(-1, 1, 1, -1, 0, 1);
    blurScene_ = threepp::Scene::create();

    auto blurGeometry = threepp::PlaneGeometry::create(2, 2);
    blurMaterial_ = threepp::MeshBasicMaterial::create();
    blurMaterial_->color = threepp::Color(0.80f, 0.20f, 0.4f);
    blurMaterial_->transparent = true;
    blurMaterial_->opacity = 0.0f;
    blurMaterial_->depthTest = false;

    auto blurPlane = threepp::Mesh::create(blurGeometry, blurMaterial_);
    blurScene_->add(blurPlane);
}

void BlurOverlay::update(float& blurLevel, float dt, float elapsedTime, float maxBlur, float decayRate) {
    blurLevel = std::max(0.0f, blurLevel - decayRate * dt);

    if (blurLevel <= 0.0f) {
        blurMaterial_->opacity = 0.0f;
        return;
    }

    float intensity = blurLevel / maxBlur;

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

void BlurOverlay::render(threepp::GLRenderer& renderer) {
    renderer.autoClear = false;
    renderer.render(*blurScene_, *orthoCamera_);
    renderer.autoClear = true;
}

