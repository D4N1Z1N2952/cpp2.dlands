#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>    // For Uint8 (keystate)
#include <vector>        // For std::vector (world)
#include "DataTypes.h"   // For Player struct, Tile struct
#include "GameConstants.h" // For PLAYER_SPEED, JUMP_FORCE, GRAVITY etc.

// Function declaration for player movement
void HandlePlayerMovement(Player& player, const Uint8* keystate, float deltaTime, const std::vector<std::vector<Tile>>& world);

#endif // PLAYER_H
