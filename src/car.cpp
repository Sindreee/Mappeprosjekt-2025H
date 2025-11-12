//
// Created by sindr on 10/16/2025.
//


#include <threepp/loaders/AssimpLoader.hpp>
#include <threepp/threepp.hpp>
#include <car.hpp>


// cpp
#include <threepp/threepp.hpp>

Car::Car() {
    group_ = std::make_shared<threepp::Group>();  // Changed from make_unique

    threepp::AssimpLoader loader;

    deerModel_ = loader.load("../Models/Mesh_Deer.obj");
    if (deerModel_) {
        deerModel_->position.set(0, 0, 0);
        deerModel_->scale.set(0.05f, 0.05f, 0.05f);
        group_->add(deerModel_);
    }

    skateboardModel_ = loader.load("../Models/Skateboard.obj");
    if (skateboardModel_) {
        skateboardModel_->scale.set(30.0f, 30.0f, 30.0f);
        skateboardModel_->rotation.y = threepp::math::PI / 2;
        skateboardModel_->position.set(0, -75.0f, 0);
        deerModel_->add(skateboardModel_);
    }
}

void Car::update(float deltaTime) {}

std::shared_ptr<threepp::Group> Car::getGroup() {
    return group_;  // No .get() needed
}