#ifndef DATATYPES_H
#define DATATYPES_H

#include <SDL2/SDL.h>
#include "GameConstants.h" // Include constants that might be used in struct initializations or default values

// Add these new structures before the existing ones
// Update BiomeType enum
enum class BiomeType {
    DEEP_WATER,
    SHALLOW_WATER,
    BEACH,
    PLAINS,
    FOREST,
    HILLS,
    MOUNTAINS,
    SNOW_CAPS
};

struct BiomeProperties {
    SDL_Color baseColor;
    float heightModifier;
    float roughness;
    bool walkable;
};

// Add this new struct for terrain generation
struct TerrainData {
    float elevation;
    float moisture;
    float riverValue;
    BiomeType biome;
};

struct Tile {
    int x, y;
    int elevation;
    SDL_Color color;
    bool walkable;
};

struct Player {
    float x = WORLD_WIDTH / 2.0f;   // Start in the middle of the world
    float y = WORLD_HEIGHT / 2.0f;
    float elevation = INITIAL_ELEVATION;
    float velocityX = 0;
    float velocityY = 0;
    float velocityZ = 0;
    bool isJumping = false;
    SDL_Color color = {255, 0, 0, 255}; // Bright red color
};

struct Camera {
    float x = 0;
    float y = 0;

    // Forward declaration of Player for the update method
    // The actual definition of Player is above, but this avoids a circular dependency
    // if Player methods were to use Camera, and Camera methods use Player.
    // However, in this simple case, it's fine as Player is fully defined before Camera.
    void update(const Player& player); // Implementation will be in Camera.cpp or main.cpp initially
};

#endif // DATATYPES_H
