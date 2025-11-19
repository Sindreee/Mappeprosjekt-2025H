#ifndef CAMERA_RIG_HPP
#define CAMERA_RIG_HPP

#include <threepp/threepp.hpp>

class CameraRig : public threepp::KeyListener { // inherit KeyListener
public:
    CameraRig(float distance = 15.0f, float height = 3.0f, float smoothing = 0.1f);

    void follow(threepp::PerspectiveCamera& camera, threepp::Object3D& target);

    // Toggle reverse view manually
    void setReverse(bool enabled) { reverseView_ = enabled; }
    bool isReverse() const { return reverseView_; }

    // Key events: hold L to reverse camera
    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

private:
    float distance_;
    float height_;
    float smoothing_;
    float smoothedRotation_ = 0.0f;
    bool reverseView_ = false; // true while L is held
};

#endif // CAMERA_RIG_HPP
