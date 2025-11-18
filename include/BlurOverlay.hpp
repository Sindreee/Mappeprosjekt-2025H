#ifndef BLUR_OVERLAY_HPP
#define BLUR_OVERLAY_HPP

#include <threepp/threepp.hpp>
#include <memory>

class BlurOverlay {
public:
    BlurOverlay();

    void update(float& blurLevel, float dt, float elapsedTime, float maxBlur, float decayRate);
    void render(threepp::GLRenderer& renderer);

private:
    std::shared_ptr<threepp::Scene> blurScene_;
    std::shared_ptr<threepp::OrthographicCamera> orthoCamera_;
    std::shared_ptr<threepp::MeshBasicMaterial> blurMaterial_;
};

#endif // BLUR_OVERLAY_HPP

