#ifndef BLUR_OVERLAY_HPP
#define BLUR_OVERLAY_HPP

#include <threepp/threepp.hpp>
#include <memory>

// Manages a full-screen blur overlay effect whose strength is driven by a logical blur level
class BlurOverlay {
public:
    BlurOverlay();

    // Update the logical blur level and compute the corresponding overlay opacity
    void update(float& blurLevel, float dt, float elapsedTime);

    // Draw the overlay on top of the existing scene using the provided renderer
    void render(threepp::GLRenderer& renderer);

private:
    std::shared_ptr<threepp::Scene> blurScene_;
    std::shared_ptr<threepp::OrthographicCamera> orthoCamera_;
    std::shared_ptr<threepp::MeshBasicMaterial> blurMaterial_;
};

#endif // BLUR_OVERLAY_HPP