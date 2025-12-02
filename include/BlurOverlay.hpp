#ifndef BLUR_OVERLAY_HPP
#define BLUR_OVERLAY_HPP

#include <threepp/threepp.hpp>
#include <memory>

// Renders a fullscreen colored quad on top of the scene to visualize the blur/intoxication level.
// The quad's opacity is driven by a logical blurLevel value.
class BlurOverlay {
public:
    BlurOverlay();

    // Update the logical blur level and compute the corresponding overlay opacity.
    // "blurLevel" is modified in place so the game can track its decay over time.
    void update(float& blurLevel, float dt, float elapsedTime);

    // Draw the overlay on top of the existing scene using the provided renderer.
    void render(threepp::GLRenderer& renderer);

private:
    std::shared_ptr<threepp::Scene> blurScene_;
    std::shared_ptr<threepp::OrthographicCamera> orthoCamera_;
    std::shared_ptr<threepp::MeshBasicMaterial> blurMaterial_;
};

#endif // BLUR_OVERLAY_HPP