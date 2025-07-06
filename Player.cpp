#include "Player.h"
#include <iostream> // For std::cout (debug messages)
#include <cmath>    // For std::abs, std::max, std::min
#include <algorithm>// For std::max, std::min (redundant if cmath is included, but common practice)

// Player movement logic
void HandlePlayerMovement(Player& player, const Uint8* keystate, float deltaTime, const std::vector<std::vector<Tile>>& world) {
    // Increase speed for better response
    float moveSpeed = PLAYER_SPEED * deltaTime * 5.0f;

    // Store previous position
    float prevX = player.x;
    float prevY = player.y;

    // Print key states for debugging
    static Uint32 lastKeyPrintTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastKeyPrintTime > 500) { // Print every 500ms
        // This can be made conditional (e.g. #ifdef DEBUG_MOVEMENT) or removed for release builds
        // std::cout << "Key states: W:" << (keystate[SDL_SCANCODE_W] ? "1" : "0")
        //           << " A:" << (keystate[SDL_SCANCODE_A] ? "1" : "0")
        //           << " S:" << (keystate[SDL_SCANCODE_S] ? "1" : "0")
        //           << " D:" << (keystate[SDL_SCANCODE_D] ? "1" : "0") << std::endl;
        lastKeyPrintTime = currentTime;
    }

    // Handle movement - isometric style with corrected directions
    if (keystate[SDL_SCANCODE_W]) {
        player.y -= moveSpeed; // Move forward (north-west in iso view if y is "up" on map)
        // std::cout << "Moving FORWARD (W)" << std::endl;
    }
    if (keystate[SDL_SCANCODE_S]) {
        player.y += moveSpeed; // Move backward (south-east in iso view)
        // std::cout << "Moving BACKWARD (S)" << std::endl;
    }
    if (keystate[SDL_SCANCODE_A]) {
        player.x -= moveSpeed; // Move left (south-west in iso view)
        // std::cout << "Moving LEFT (A)" << std::endl;
    }
    if (keystate[SDL_SCANCODE_D]) {
        player.x += moveSpeed; // Move right (north-east in iso view)
        // std::cout << "Moving RIGHT (D)" << std::endl;
    }

    // Keep player within world bounds with some margin
    // Using 1.0f as margin, can be adjusted or made a constant
    player.x = std::max(1.0f, std::min(player.x, static_cast<float>(WORLD_WIDTH - 2)));
    player.y = std::max(1.0f, std::min(player.y, static_cast<float>(WORLD_HEIGHT - 2)));

    // Check if new position is walkable and adjust elevation
    int tileX = static_cast<int>(player.x);
    int tileY = static_cast<int>(player.y);

    if (tileX >= 0 && tileX < WORLD_WIDTH && tileY >= 0 && tileY < WORLD_HEIGHT) {
        const Tile& currentTile = world[tileY][tileX];

        // Original code had 'false && !currentTile.walkable' effectively disabling walkability check
        // Keeping that logic for now, meaning player can walk anywhere.
        // TODO: Implement proper walkability check based on currentTile.walkable
        if (false && !currentTile.walkable) {
            player.x = prevX;
            player.y = prevY;
            // std::cout << "Blocked by non-walkable tile" << std::endl;
        } else {
            // Adjust player elevation to match the terrain
            if (!player.isJumping) {
                float targetElevation = static_cast<float>(currentTile.elevation);
                // Smoothly interpolate to terrain height
                player.elevation += (targetElevation - player.elevation) * 0.2f;
            }
        }
    }

    // Handle jumping
    if (keystate[SDL_SCANCODE_SPACE] && !player.isJumping) {
        player.velocityZ = JUMP_FORCE;
        player.isJumping = true;
        // std::cout << "Jumping!" << std::endl;
    }

    // Apply gravity and update elevation if jumping
    if (player.isJumping) {
        player.velocityZ -= GRAVITY;
        player.elevation += player.velocityZ;

        // Determine terrain height at current player (x,y) for landing detection
        int currentTileX = static_cast<int>(player.x);
        int currentTileY = static_cast<int>(player.y);
        float terrainHeight = INITIAL_ELEVATION; // Default if out of bounds

        if (currentTileX >= 0 && currentTileX < WORLD_WIDTH && currentTileY >= 0 && currentTileY < WORLD_HEIGHT) {
            terrainHeight = static_cast<float>(world[currentTileY][currentTileX].elevation);
        }

        // Check for landing
        if (player.elevation <= terrainHeight) {
            player.elevation = terrainHeight;
            player.velocityZ = 0;
            player.isJumping = false;
            // std::cout << "Landed" << std::endl;
        }
    }

    // Debug print for player position changes (can be conditional)
    static float lastPrintX = 0, lastPrintY = 0, lastPrintZ = 0;
    if (std::abs(player.x - lastPrintX) > 0.1f || std::abs(player.y - lastPrintY) > 0.1f || std::abs(player.elevation - lastPrintZ) > 0.1f) {
        // std::cout << "Player moved to: (" << player.x << ", " << player.y << ", " << player.elevation << ")" << std::endl;
        lastPrintX = player.x;
        lastPrintY = player.y;
        lastPrintZ = player.elevation;
    }
}
