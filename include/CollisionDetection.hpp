//
// Created by sindr on 11/10/2025.
//

#ifndef MAPPEOPPGAVE_COLLISIONDETECTION_HPP
#define MAPPEOPPGAVE_COLLISIONDETECTION_HPP



#include <threepp/threepp.hpp>
#include <vector>

class CollisionDetection {
public:
    CollisionDetection(float collisionRadius = 7.0f);

    void setObstacles(const std::vector<threepp::Vector3>& positions);
    void addObstacle(const threepp::Vector3& position);
    bool checkCollision(const threepp::Vector3& position) const;

private:
    std::vector<threepp::Vector3> obstaclePositions_;
    float collisionRadius_;
};


#endif //MAPPEOPPGAVE_COLLISIONDETECTION_HPP