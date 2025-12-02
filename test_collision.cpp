// test_collision.cpp
#include <catch2/catch_test_macros.hpp>
#include "CollisionDetection.hpp"
#include <threepp/threepp.hpp>

TEST_CASE("Collision Detection Logic", "[Collision]") {
    CollisionDetection cd;

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
        cd.addCollider(obj1, 2.0f, ColliderType::TREE);

        // Tree radius (2) + Check radius (1) = 3. Distance is 0.5. Collision!
        bool blocked = cd.isBlocked(threepp::Vector3(0.5f, 0, 0), 1.0f);
        REQUIRE(blocked == true);

        // Distance is 10. No Collision.
        bool free = cd.isBlocked(threepp::Vector3(10.0f, 0, 0), 1.0f);
        REQUIRE(free == false);
    }

    SECTION("isBlocked ignores specified object (Self-Collision check)") {
        cd.addCollider(obj1, 2.0f, ColliderType::CAR);
        bool blocked = cd.isBlocked(threepp::Vector3(0,0,0), 1.0f, obj1.get());
        REQUIRE(blocked == false);
    }
}

TEST_CASE("Collision callbacks and cleanup work", "[Collision]") {
    CollisionDetection cd;
    auto carObj = threepp::Object3D::create();
    auto bearObj = threepp::Object3D::create();
    carObj->position.set(0, 0, 0);
    bearObj->position.set(1, 0, 0);

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

        carHit = false; bearHit = false;
        bearObj->position.set(1000, 0, 0);

        cd.processAllPairs();
        REQUIRE(carHit == false);
        REQUIRE(bearHit == false);
    }
}