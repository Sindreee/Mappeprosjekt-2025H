#ifndef CAMERA_RIG_HPP
#define CAMERA_RIG_HPP

#include <threepp/threepp.hpp>

// camera rig inherits from KeyListener to track key state for reverse view
class CameraRig : public threepp::KeyListener {

public:

    // Create a camera rig with the given parameters.
    CameraRig(float distance = 23.0f, float height = 6.0f, float smoothing = 0.1f);

    // Update the camera position and orientation to follow the target object.
    void follow(threepp::PerspectiveCamera& camera, threepp::Object3D& target);


    // Lets other parts of the program know if rearwiew is currently active or not
    void setReverse(bool enabled) { reverseView_ = enabled; }
    bool isReverse() const { return reverseView_; }

    // keylistener for the rearwiew
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
