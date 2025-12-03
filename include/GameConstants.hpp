#ifndef GAMECONSTANTS_HPP
#define GAMECONSTANTS_HPP

// Global tuning parameters and gameplay constants used across the game.
namespace Config {
    // Size and physics of the world
    constexpr float WORLD_SIZE = 2000.0f;
    constexpr float WORLD_BOUNDS = 1000.0f;
    constexpr float GRAVITY = 9.81f;

    // Car configuration
    constexpr float CAR_BASE_SPEED = 150.0f;
    constexpr int CAR_MAX_HEALTH = 10;
    constexpr float CAR_COLLISION_RADIUS = 7.0f;
    constexpr float CAR_HIT_COOLDOWN = 0.5f;

    // World population counts
    constexpr int TREE_COUNT = 150;
    constexpr int MUSHROOM_COUNT = 50;
    constexpr int BEAR_COUNT = 70;
    constexpr int WOLF_COUNT_BASE = 12;

    // Gameplay effects
    constexpr float MUSHROOM_SPEED_BOOST = 7.0f;
    constexpr float MAX_BLUR = 12.0f;
}

#endif // GAMECONSTANTS_HPP