#include "DataTypes.h"    // For Camera and Player struct definitions
#include "GameConstants.h"  // For TILE_WIDTH, TILE_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT
#include <SDL2/SDL.h>       // For SDL_GetTicks, Uint32
#include <iostream>         // For std::cout (debug)

// Implementation for Camera::update method
void Camera::update(const Player& player) {
    // Convert player world position to screen space without camera offset
    float screenX_player_world = (player.x - player.y) * (TILE_WIDTH / 2.0f);
    float screenY_player_world = (player.x + player.y) * (TILE_HEIGHT / 2.0f) - player.elevation;

    // Immediately set camera to center player (no smoothing)
    // This ensures player is always perfectly centered
    x = screenX_player_world - (SCREEN_WIDTH / 2.0f); // Use floating point division
    y = screenY_player_world - (SCREEN_HEIGHT / 2.0f); // Use floating point division

    // Debug - print less frequently to avoid console spam
    static Uint32 lastCameraDebugTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastCameraDebugTime > 1000) { // Once per second
        // std::cout << "Camera position (from Camera.cpp): (" << x << ", " << y << ")" << std::endl;
        lastCameraDebugTime = currentTime;
    }
}
