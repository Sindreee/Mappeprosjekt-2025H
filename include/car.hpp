// cpp
#ifndef CAR_HPP
#define CAR_HPP

#include <threepp/threepp.hpp>



#include <threepp/threepp.hpp>
#include <memory>



#include <threepp/threepp.hpp>
#include <memory>

class Car {
public:
    Car();

    void update(float deltaTime);
    std::shared_ptr<threepp::Group> getGroup();  // Changed return type

private:
    std::shared_ptr<threepp::Group> group_;  // Changed from unique_ptr
    std::shared_ptr<threepp::Object3D> deerModel_;
    std::shared_ptr<threepp::Object3D> skateboardModel_;
};

#endif // CAR_HPP




