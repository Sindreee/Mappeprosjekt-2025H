#ifndef CAMERA_RIG_HPP
#define CAMERA_RIG_HPP

#include <threepp/threepp.hpp>

class CameraRig {
public:
    CameraRig(float distance = 15.0f, float height = 3.0f, float smoothing = 0.1f);

    void follow(threepp::PerspectiveCamera& camera, threepp::Object3D& target);

private:
    float distance_;
    float height_;
    float smoothing_;
    float smoothedRotation_ = 0.0f;
};

#endif // CAMERA_RIG_HPP

