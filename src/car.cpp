//
// Created by sindr on 10/16/2025.
//


#include <threepp/loaders/AssimpLoader.hpp>
#include <threepp/threepp.hpp>
#include <car.hpp>
#include <iostream>


Car::Car() {
    group_ = std::make_shared<threepp::Group>();

    threepp::AssimpLoader loader;

    deerModel_ = loader.load("../Models/Mesh_Deer.obj");
    if (deerModel_) {
        deerModel_->position.set(0, 0, 0);
        deerModel_->scale.set(0.05f, 0.05f, 0.05f);
        group_->add(deerModel_);
    }

        skateboardModel_ = loader.load("../Models/SkateboardBrettOgAkslinger2.obj");
        if (skateboardModel_) {
            skateboardModel_->scale.set(0.5f, 0.5f, 0.5f);
            skateboardModel_->position.set(0, -65.0f, 0);
            deerModel_->add(skateboardModel_);

            auto frontLeftWheel = loader.load("../Models/TerrainWheel3.obj");
            if (frontLeftWheel) {
                frontLeftWheel->position.set(70, -8, 70);
                wheels_[0] = frontLeftWheel;
                skateboardModel_->add(wheels_[0]);
            } else {
                std::cerr << "Failed to load front left wheel" << std::endl;
            }

            auto frontRightWheel = loader.load("../Models/TerrainWheel3.obj");
            if (frontRightWheel) {
                frontRightWheel->position.set(-70, -8, 70);
                wheels_[1] = frontRightWheel;
                skateboardModel_->add(wheels_[1]);
            }

            auto backLeftWheel = loader.load("../Models/TerrainWheel3.obj");
            if (backLeftWheel) {
                backLeftWheel->position.set(70, -8, -70);
                wheels_[2] = backLeftWheel;
                skateboardModel_->add(wheels_[2]);
            }

            auto backRightWheel = loader.load("../Models/TerrainWheel3.obj");
            if (backRightWheel) {
                backRightWheel->position.set(-70, -8, -70);
                wheels_[3] = backRightWheel;
                skateboardModel_->add(wheels_[3]);
            }
        }
}


void Car::setSpeed(float speed) {
    speed_ = speed;
}

void Car::setRotation(float rotation) {
    rotation_ = rotation;
}

void Car::update(float deltaTime) {
    // Calculate base wheel rotation from forward/backward speed
    float speedRotation = speed_ * deltaTime * 0.1f;

    // Calculate rotation from turning (tank-like controls)
    float turnRotation = rotation_ * deltaTime * 0.05f;

    // Left wheels: forward speed minus turn (turning subtracts from left)
    float leftWheelDelta = speedRotation - turnRotation;

    // Right wheels: forward speed plus turn (turning adds to right)
    float rightWheelDelta = speedRotation + turnRotation;

    // Update cumulative rotation
    wheelRotation_ += speedRotation;

    // Apply to wheels
    if (wheels_[0]) wheels_[0]->rotation.x += leftWheelDelta;   // Front left
    if (wheels_[2]) wheels_[2]->rotation.x += leftWheelDelta;   // Back left
    if (wheels_[1]) wheels_[1]->rotation.x += rightWheelDelta;  // Front right
    if (wheels_[3]) wheels_[3]->rotation.x += rightWheelDelta;  // Back right
}

std::shared_ptr<threepp::Group> Car::getGroup() {
    return group_;
}