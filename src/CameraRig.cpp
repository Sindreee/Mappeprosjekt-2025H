#include "CameraRig.hpp"
#include <cmath>

CameraRig::CameraRig(float distance, float height, float smoothing)
    : distance_(distance), height_(height), smoothing_(smoothing), reverseView_(false) {}

void CameraRig::follow(threepp::PerspectiveCamera& camera, threepp::Object3D& target) {
    smoothedRotation_ += (target.rotation.y - smoothedRotation_) * smoothing_;

    float effectiveRotation = smoothedRotation_ + (reverseView_ ? threepp::math::PI : 0.0f);

    float offsetX = distance_ * std::sin(effectiveRotation);
    float offsetZ = distance_ * std::cos(effectiveRotation);

    camera.position.x = target.position.x - offsetX;
    camera.position.y = target.position.y + height_;
    camera.position.z = target.position.z - offsetZ;

    camera.lookAt(target.position);
}

void CameraRig::onKeyPressed(threepp::KeyEvent evt) {
    if (evt.key == threepp::Key::L) {
        reverseView_ = true;
    }
}

void CameraRig::onKeyReleased(threepp::KeyEvent evt) {
    if (evt.key == threepp::Key::L) {
        reverseView_ = false;
    }
}
