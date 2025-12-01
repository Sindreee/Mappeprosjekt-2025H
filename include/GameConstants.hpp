//
// Created by sindr on 11/30/2025.
//

#ifndef MAPPEOPPGAVE_GAMECONSTANTS_HPP
#define MAPPEOPPGAVE_GAMECONSTANTS_HPP


namespace Config {
    // World Settings
    constexpr float WORLD_SIZE = 3000.0f;
    constexpr float WORLD_BOUNDS = 1500.0f; // +/- denne verdien
    constexpr float GRAVITY = 9.81f;

    // Car Settings
    constexpr float CAR_BASE_SPEED = 150.0f;
    constexpr int CAR_MAX_HEALTH = 10;
    constexpr float CAR_COLLISION_RADIUS = 7.0f;
    constexpr float CAR_HIT_COOLDOWN = 0.5f;

    // Entity Counts
    constexpr int TREE_COUNT = 1000;
    constexpr int MUSHROOM_COUNT = 300;
    constexpr int BEAR_COUNT = 400;
    constexpr int WOLF_COUNT_BASE = 13; // Level 1

    // Gameplay
    constexpr float MUSHROOM_SPEED_BOOST = 7.0f; // Fart per blur level
    constexpr float MAX_BLUR = 12.0f;
}


#endif //MAPPEOPPGAVE_GAMECONSTANTS_HPP