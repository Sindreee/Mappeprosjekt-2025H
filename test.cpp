// This define tells Catch2 to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp> // For floating point comparisons

// Include project headers
#include "GameConstants.hpp"
#include "CollisionDetection.hpp"
#include "car.hpp"
#include "Wolf.hpp"
#include "Bear.hpp"
#include "GameObject.hpp"
#include "Random.hpp"

#include <threepp/threepp.hpp>

// --- TEST 1: Math and Constants ---
TEST_CASE("Game Constants are sane", "[Config]") {
    REQUIRE(Config::CAR_MAX_HEALTH > 0);
    REQUIRE(Config::WORLD_SIZE > Config::WORLD_BOUNDS);
    REQUIRE(Config::GRAVITY > 0.0f);
}

TEST_CASE("Random Generator produces values in range", "[Random]") {
    // We run this multiple times to ensure stability
    for(int i=0; i<10; ++i) {
        float val = rnd::uniform(0.0f, 10.0f);
        REQUIRE(val >= 0.0f);
        REQUIRE(val <= 10.0f);
    }
}

// --- TEST 2: Collision System Logic ---
// We test this without loading 3D models by using empty Object3D containers.
TEST_CASE("Collision Detection Logic", "[Collision]") {

    // ENDRING HER: Fjernet 5.0f. Bruk standard constructor.
    CollisionDetection cd;

    // Create dummy objects to represent entities in the world
    auto obj1 = threepp::Object3D::create();
    obj1->position.set(0, 0, 0);

    auto obj2 = threepp::Object3D::create();
    obj2->position.set(10, 0, 0);

    SECTION("Adding colliders works") {
        int id1 = cd.addCollider(obj1, 2.0f, ColliderType::TREE);
        int id2 = cd.addCollider(obj2, 2.0f, ColliderType::CAR);
        REQUIRE(id1 == 0);
        REQUIRE(id2 == 1);
    }

    SECTION("isBlocked detects collision correctly") {
        // Add a tree at (0,0,0) with radius 2
        cd.addCollider(obj1, 2.0f, ColliderType::TREE);

        // Check a point very close to the tree (should be blocked)
        // Tree radius (2) + Check radius (1) = 3. Distance is 0.5. Collision!
        bool blocked = cd.isBlocked(threepp::Vector3(0.5f, 0, 0), 1.0f);
        REQUIRE(blocked == true);

        // Check a point far away (should be free)
        // Distance is 10. Collision radius sum is 3. No collision.
        bool free = cd.isBlocked(threepp::Vector3(10.0f, 0, 0), 1.0f);
        REQUIRE(free == false);
    }

    SECTION("isBlocked ignores specified object (Self-Collision check)") {
        // This simulates the car checking if it can move.
        // It shouldn't collide with itself.
        cd.addCollider(obj1, 2.0f, ColliderType::CAR);

        // Even though we are INSIDE obj1, passing obj1 as "ignoreObject" should return false
        bool blocked = cd.isBlocked(threepp::Vector3(0,0,0), 1.0f, obj1.get());
        REQUIRE(blocked == false);
    }
}
// --- TEST 3: Car Mechanics ---
TEST_CASE("Car Gameplay Logic", "[Car]") {
    // The Car constructor tries to load models. Even if files are missing
    // (which might happen in a CI test environment), your code safely handles
    // null models, so we can still test the logic (health, speed).
    Car car;

    SECTION("Initial State") {
        REQUIRE(car.getHealth() == Config::CAR_MAX_HEALTH);
        REQUIRE(car.isDead() == false);
        REQUIRE(car.isFalling() == false);
    }

    SECTION("Damage System") {
        int initialHealth = car.getHealth();

        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);

        // Test Invulnerability (Hit Cooldown)
        // Taking damage immediately again shouldn't lower health due to cooldown
        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);
    }

    SECTION("Death Logic") {
        // Deal lethal damage
        car.takeDamage(100);
        REQUIRE(car.isDead() == true);
        REQUIRE(car.getHealth() <= 0);
    }

    SECTION("Speed System") {
        float base = Config::CAR_BASE_SPEED;

        // Test Speed Boost (Mushrooms)
        car.applySpeedBoost(50.0f);
        REQUIRE(car.getMaxSpeed() == Catch::Approx(base + 50.0f));

        // Test Speed Penalty (Bears)
        car.reset(); // Reset to clear boost
        car.modifyBaseSpeed(-20.0f);
        REQUIRE(car.getMaxSpeed() == Catch::Approx(base - 20.0f));
    }
}

// --- TEST 4: Wolf AI State ---
TEST_CASE("Wolf Entity Logic", "[Wolf]") {
    // Create a dummy mesh for the wolf
    auto mesh = threepp::Object3D::create();
    threepp::Vector3 offset(0,0,0);

    // Create a wolf: MaxSpeed 100, Acc 50
    Wolf wolf(mesh, 100.0f, 50.0f, offset);

    SECTION("Initial State") {
        REQUIRE(wolf.isAlive() == true);
        REQUIRE(wolf.getHits() == 0);
    }

    SECTION("Damage Response") {
        wolf.takeDamage();
        REQUIRE(wolf.getHits() == 1);

        // Taking damage triggers the wolf to die after 'hitsToDie_' (which is 5 in your code)
        wolf.takeDamage();
        wolf.takeDamage();
        wolf.takeDamage();
        wolf.takeDamage(); // 5th hit

        REQUIRE(wolf.isAlive() == false);
    }

    SECTION("Visuals hidden on death") {
        // Kill the wolf
        for(int i=0; i<6; ++i) wolf.takeDamage();

        // The mesh should be hidden
        REQUIRE(mesh->visible == false);
    }
}
// --- TEST 5: Bear AI Logic ---
TEST_CASE("Bear Entity Logic", "[Bear]") {
    // Create a dummy mesh so we don't need to load external assets
    auto mesh = threepp::Object3D::create();
    mesh->position.set(0, 0, 0);

    // Instantiate a Bear with the dummy mesh
    Bear bear(mesh);

    SECTION("Bear moves over time") {
        // Initial position is (0, 0, 0) due to our dummy setup
        // (Note: The constructor randomizes direction, but position starts at mesh pos)

        // Simulate 1 second of game time
        bear.update(1.0f);

        // Check that the bear has moved from the origin
        // We check if X or Z is non-zero.
        bool hasMoved = (std::abs(mesh->position.x) > 0.0f || std::abs(mesh->position.z) > 0.0f);
        REQUIRE(hasMoved == true);
    }
}

// --- TEST 6: Blur Mechanic Decay ---
TEST_CASE("Blur Decay Logic", "[Blur]") {
    // We want to test the math behind the blur decay without creating the full overlay object
    // (creating the object might require OpenGL context).
    // The logic from BlurOverlay::update is essentially:
    // blurLevel = std::max(0.0f, blurLevel - decayRate * dt);

    float blurLevel = 10.0f; // Start with high intoxication
    float decayRate = 0.2f;  // Must match the constant in your BlurOverlay.cpp (or be passed in)
    float dt = 1.0f;         // 1 second passes

    SECTION("Blur decreases over time") {
        // Simulate logic manually to verify the formula works as intended
        // (Ideally, we would refactor BlurOverlay to separate logic from rendering,
        // but testing the formula here validates the game design).

        float newLevel = std::max(0.0f, blurLevel - decayRate * dt);

        REQUIRE(newLevel < blurLevel);
        REQUIRE(newLevel == Catch::Approx(9.8f)); // 10.0 - 0.2 * 1.0
    }

    SECTION("Blur clamps to zero") {
        blurLevel = 0.1f;
        dt = 10.0f; // Long time passes

        float newLevel = std::max(0.0f, blurLevel - decayRate * dt);
        REQUIRE(newLevel == 0.0f); // Should not go negative
    }
}

// --- Additional Tests: Car Movement, Falling, and Reset ---
TEST_CASE("Car movement respects max speed and falling/reset behavior", "[Car]") {
    Car car;

    SECTION("Forward movement does not exceed max speed") {
        // Arrange: give the car a big speed boost
        car.applySpeedBoost(50.0f);
        float maxSpeed = car.getMaxSpeed();

        auto group = car.getGroup();
        group->position.set(0, 0, 0);

        float dt = 0.5f; // half a second
        car.setSpeed(maxSpeed * 10.0f); // try to exceed max speed internally

        // Act: update once
        car.update(dt);

        // Assert: displacement along XZ should not be absurdly large
        float distXZ = std::sqrt(group->position.x * group->position.x + group->position.z * group->position.z);
        REQUIRE(distXZ <= Catch::Approx(maxSpeed * dt).epsilon(0.5)); // allow some tolerance
    }

    SECTION("Hit cooldown expires after enough time") {
        int initialHealth = car.getHealth();

        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);

        // Immediately hitting again should not reduce health because of cooldown
        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 1);

        // Advance time beyond cooldown and try again
        car.update(Config::CAR_HIT_COOLDOWN * 1.1f);
        car.takeDamage(1);
        REQUIRE(car.getHealth() == initialHealth - 2);
    }

    SECTION("Car starts falling when leaving world bounds") {
        auto group = car.getGroup();
        group->position.set(Config::WORLD_BOUNDS + 10.0f, 0, 0); // clearly out of bounds

        REQUIRE(car.isFalling() == false);
        float y0 = group->position.y;

        car.update(0.1f);

        REQUIRE(car.isFalling() == true);
        float y1 = group->position.y;

        // After gravity step, Y should have decreased
        REQUIRE(y1 < y0);
    }

    SECTION("Reset restores default state") {
        auto group = car.getGroup();
        group->position.set(100, 5, -50);
        car.applySpeedBoost(100.0f);
        car.modifyBaseSpeed(-10.0f);
        car.takeDamage(10);

        car.reset();

        REQUIRE(car.getHealth() == Config::CAR_MAX_HEALTH);
        REQUIRE(car.isDead() == false);
        REQUIRE(car.isFalling() == false);
        REQUIRE(car.getMaxSpeed() == Catch::Approx(Config::CAR_BASE_SPEED));
        REQUIRE(group->position.x == Catch::Approx(0.0f));
        REQUIRE(group->position.y == Catch::Approx(0.0f));
        REQUIRE(group->position.z == Catch::Approx(0.0f));
    }
}

// --- Additional Tests: Collision callbacks and cleanup ---
TEST_CASE("Collision callbacks and cleanup work", "[Collision]") {
    CollisionDetection cd;

    auto carObj = threepp::Object3D::create();
    auto bearObj = threepp::Object3D::create();

    carObj->position.set(0, 0, 0);
    bearObj->position.set(1, 0, 0); // within sum of radii below

    bool carHit = false;
    bool bearHit = false;

    cd.addCollider(carObj, 1.0f, ColliderType::CAR,
                   [&](const Collider&, const Collider&) { carHit = true; });
    cd.addCollider(bearObj, 1.0f, ColliderType::BEAR,
                   [&](const Collider&, const Collider&) { bearHit = true; });

    cd.processAllPairs();

    REQUIRE(carHit == true);
    REQUIRE(bearHit == true);

    SECTION("Inactive colliders are removed by cleanup") {
        cd.removeCollidersOfType(ColliderType::BEAR);
        cd.cleanupInactive();

        // The previous bear collider should no longer participate, so no hits when moved elsewhere
        carHit = false;
        bearHit = false;
        bearObj->position.set(1000, 0, 0);

        cd.processAllPairs();
        REQUIRE(carHit == false);
        REQUIRE(bearHit == false);
    }
}

// --- Additional Tests: Random helpers ---
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

// --- Additional Tests: Wolf movement and damage scaling ---
TEST_CASE("Wolf movement and damage scaling behavior", "[Wolf]") {
    auto mesh = threepp::Object3D::create();
    mesh->position.set(0, 0, 0);
    threepp::Vector3 offset(0, 0, 0);

    Wolf wolf(mesh, 100.0f, 50.0f, offset);

    SECTION("Wolf moves closer to target over time") {
        threepp::Vector3 target(200.0f, 0.0f, 0.0f);

        auto dist = [&](const threepp::Vector3& a, const threepp::Vector3& b) {
            float dx = a.x - b.x;
            float dz = a.z - b.z;
            return std::sqrt(dx * dx + dz * dz);
        };

        float d0 = dist(mesh->position, target);

        for (int i = 0; i < 50; ++i) {
            wolf.update(0.1f, target);
        }

        float d1 = dist(mesh->position, target);
        REQUIRE(d1 < d0);
    }

    SECTION("Wolf stops moving when dead") {
        threepp::Vector3 target(200.0f, 0.0f, 0.0f);

        for (int i = 0; i < 6; ++i) wolf.takeDamage();
        REQUIRE(wolf.isAlive() == false);

        threepp::Vector3 before = mesh->position;
        wolf.update(1.0f, target);
        threepp::Vector3 after = mesh->position;

        REQUIRE(before.x == Catch::Approx(after.x));
        REQUIRE(before.z == Catch::Approx(after.z));
    }
}

// --- Additional Tests: Bear wandering behavior ---
TEST_CASE("Bear wandering timer changes direction over time", "[Bear]") {
    auto mesh = threepp::Object3D::create();
    mesh->position.set(0, 0, 0);
    Bear bear(mesh);

    auto dirFromStep = [&](float dt) {
        float x0 = mesh->position.x;
        float z0 = mesh->position.z;
        bear.update(dt);
        float dx = mesh->position.x - x0;
        float dz = mesh->position.z - z0;
        return std::make_pair(dx, dz);
    };

    // First small step to get an initial direction
    auto d1 = dirFromStep(0.1f);

    // Simulate enough time to cross the 5 second threshold for changing direction
    for (int i = 0; i < 60; ++i) {
        bear.update(0.1f);
    }

    auto d2 = dirFromStep(0.1f);

    // Directions should not be almost identical (within a tiny dot product threshold)
    float dot = d1.first * d2.first + d1.second * d2.second;
    float mag1 = std::sqrt(d1.first * d1.first + d1.second * d1.second);
    float mag2 = std::sqrt(d2.first * d2.first + d2.second * d2.second);

    if (mag1 > 0.0f && mag2 > 0.0f) {
        float cosAngle = dot / (mag1 * mag2);
        REQUIRE(cosAngle < 0.999f); // not almost perfectly aligned
    }
}
