#include "CameraRig.hpp"
#include <cmath>

CameraRig::CameraRig(float distance, float height, float smoothing)
    : distance_(distance), height_(height), smoothing_(smoothing) {}

void CameraRig::follow(threepp::PerspectiveCamera& camera, threepp::Object3D& target) {
    smoothedRotation_ += (target.rotation.y - smoothedRotation_) * smoothing_;

    float offsetX = distance_ * std::sin(smoothedRotation_);
    float offsetZ = distance_ * std::cos(smoothedRotation_);

    camera.position.x = target.position.x - offsetX;
    camera.position.y = target.position.y + height_;
    camera.position.z = target.position.z - offsetZ;

    camera.lookAt(target.position);
}

