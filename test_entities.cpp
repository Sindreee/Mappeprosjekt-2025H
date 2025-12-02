// test_entities.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <threepp/threepp.hpp>

#include "car.hpp"
#include "Wolf.hpp"
#include "Bear.hpp"
#include "GameConstants.hpp"

// --- TEST: Car ---
TEST_CASE("Car Gameplay Logic", "[Car]") {
    Car car;

    SECTION("Initial State") {
        REQUIRE(car.getHealth() == Config::CAR_MAX_HEALTH);
        REQUIRE(car.isDead() == false);
    }

    SECTION("Damage System") {
        int initialHealth = car.getHealth();
        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);

        // Invulnerability check
        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);
    }

    SECTION("Speed System") {
        float base = Config::CAR_BASE_SPEED;
        car.applySpeedBoost(50.0f);
        REQUIRE(car.getMaxSpeed() == Catch::Approx(base + 50.0f));
    }

    SECTION("Reset restores default state") {
        car.takeDamage(10);
        car.reset();
        REQUIRE(car.getHealth() == Config::CAR_MAX_HEALTH);
        REQUIRE(car.isDead() == false);
    }
}

// --- TEST: Wolf ---
TEST_CASE("Wolf Entity Logic", "[Wolf]") {
    auto mesh = threepp::Object3D::create();
    Wolf wolf(mesh, 100.0f, 50.0f, threepp::Vector3(0,0,0));

    SECTION("Damage Response") {
        wolf.takeDamage();
        REQUIRE(wolf.getHits() == 1);

        for(int i=0; i<4; ++i) wolf.takeDamage(); // Kill it
        REQUIRE(wolf.isAlive() == false);
        REQUIRE(mesh->visible == false);
    }

    SECTION("Wolf moves closer to target") {
        threepp::Vector3 target(200.0f, 0, 0);
        float startDist = mesh->position.distanceTo(target);

        for (int i = 0; i < 50; ++i) wolf.update(0.1f, target);

        REQUIRE(mesh->position.distanceTo(target) < startDist);
    }
}

// --- TEST: Bear ---
TEST_CASE("Bear Entity Logic", "[Bear]") {
    auto mesh = threepp::Object3D::create();
    Bear bear(mesh);

    SECTION("Bear moves over time") {
        bear.update(1.0f);
        bool hasMoved = (std::abs(mesh->position.x) > 0.0f || std::abs(mesh->position.z) > 0.0f);
        REQUIRE(hasMoved == true);
    }
}