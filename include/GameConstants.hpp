#ifndef GAMECONSTANTS_HPP
#define GAMECONSTANTS_HPP

// Global tuning parameters and gameplay constants used across the game.
namespace Config {
    // Size and physics of the world
    constexpr float WORLD_SIZE = 2000.0f;      // Size of the ground plane in world units
    constexpr float WORLD_BOUNDS = 1000.0f;    // Soft limit before the car starts to fall
    constexpr float GRAVITY = 9.81f;           // Downward acceleration when falling

    // Car configuration
    constexpr float CAR_BASE_SPEED = 150.0f;   // Default max forward speed before any boosts
    constexpr int CAR_MAX_HEALTH = 10;         // Number of hits the car can take
    constexpr float CAR_COLLISION_RADIUS = 7.0f; // Radius used when registering the car collider
    constexpr float CAR_HIT_COOLDOWN = 0.5f;   // Minimum time between two damage hits

    // World population counts
    constexpr int TREE_COUNT = 150;           // Number of decorative trees
    constexpr int MUSHROOM_COUNT = 50;        // Number of collectible mushrooms
    constexpr int BEAR_COUNT = 70;            // Number of roaming bears
    constexpr int WOLF_COUNT_BASE = 12;        // Base number of wolves (scaled by difficulty)

    // Gameplay effects
    constexpr float MUSHROOM_SPEED_BOOST = 7.0f; // Extra car speed per blur level step from mushrooms
    constexpr float MAX_BLUR = 12.0f;            // Upper bound for blur intensity
}

#endif // GAMECONSTANTS_HPP