// test_core.cpp
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "GameConstants.hpp"
#include "Random.hpp"
#include <algorithm> // For std::max used in blur test

// TEST: Game Configuration
TEST_CASE("Game Constants are sane", "[Config]") {
    REQUIRE(Config::CAR_MAX_HEALTH > 0);
    REQUIRE(Config::WORLD_SIZE > Config::WORLD_BOUNDS);
    REQUIRE(Config::GRAVITY > 0.0f);
}

// TEST: Random Number Generation
TEST_CASE("Random Generator produces values in range", "[Random]") {
    for(int i=0; i<10; ++i) {
        float val = rnd::uniform(0.0f, 10.0f);
        REQUIRE(val >= 0.0f);
        REQUIRE(val <= 10.0f);
    }
}

TEST_CASE("Random helpers stay within expected ranges", "[Random]") {
    for (int i = 0; i < 20; ++i) {
        float u = rnd::unit();
        REQUIRE(u >= 0.0f);
        REQUIRE(u <= 1.0f);

        auto p = rnd::uniform2(-5.0f, 5.0f);
        REQUIRE(p.first >= -5.0f);
        REQUIRE(p.first <= 5.0f);
        REQUIRE(p.second >= -5.0f);
        REQUIRE(p.second <= 5.0f);
    }
}

// TEST: Blur Logic (Math only)
TEST_CASE("Blur Decay Logic", "[Blur]") {
    float blurLevel = 10.0f;
    float decayRate = 0.2f;
    float dt = 1.0f;

    SECTION("Blur decreases over time") {
        float newLevel = std::max(0.0f, blurLevel - decayRate * dt);
        REQUIRE(newLevel < blurLevel);
        REQUIRE(newLevel == Catch::Approx(9.8f));
    }

    SECTION("Blur clamps to zero") {
        blurLevel = 0.1f;
        dt = 10.0f;
        float newLevel = std::max(0.0f, blurLevel - decayRate * dt);
        REQUIRE(newLevel == 0.0f);
    }
}