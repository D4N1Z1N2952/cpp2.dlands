#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <vector>
#include "DataTypes.h"
#include "GameConstants.h"

// Function declaration for player movement
void HandlePlayerMovement(Player& player, const Uint8* keystate, float deltaTime, const std::vector<std::vector<Tile>>& world);

#endif // PLAYER_H
