#ifndef CAR_HPP
#define CAR_HPP
#include <threepp/threepp.hpp>
#include <memory>
#include <array>


class Car {
public:
    Car();


    void setSpeed(float speed);
    void setRotation(float rotation);
    void update(float deltaTime);

    std::shared_ptr<threepp::Group> getGroup();
    threepp::Vector3 getPosition() const;
    void reset(); // reset to initial state for restart

private:
    std::shared_ptr<threepp::Group> group_;
    std::shared_ptr<threepp::Object3D> deerModel_;
    std::shared_ptr<threepp::Object3D> skateboardModel_;
    std::array<std::shared_ptr<threepp::Object3D>, 4> wheels_;

    float speed_ = 0.0f;
    float rotation_ = 0.0f;
    threepp::Vector3 spawnPos_{0,0,0};
};

#endif // CAR_HPP
