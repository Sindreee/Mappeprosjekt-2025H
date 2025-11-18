#ifndef CAR_HPP
#define CAR_HPP


#include <threepp/threepp.hpp>
#include <memory>
#include <array>

class Car {
public:
    Car();

    void update(float deltaTime);
    void setSpeed(float speed);
    void setRotation(float rotation);
    std::shared_ptr<threepp::Group> getGroup();
    threepp::Vector3 getPosition() const;

private:
    std::shared_ptr<threepp::Group> group_;
    std::shared_ptr<threepp::Object3D> deerModel_;
    std::shared_ptr<threepp::Object3D> skateboardModel_;
    std::array<std::shared_ptr<threepp::Object3D>, 4> wheels_;

    float speed_ = 0.0f;
    float rotation_ = 0.0f;
    float wheelRotation_ = 0.0f;
};

#endif // CAR_HPP

