#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "DataTypes.h" // For Tile, Player, Camera
#include "GameConstants.h" // For SCREEN_WIDTH, SCREEN_HEIGHT, TILE_WIDTH etc.

// Forward declare Player and Camera if their full definitions are not strictly needed in this header
// However, they are used as parameters by const reference, so full definition via DataTypes.h is fine.

// Function declarations for rendering operations
void WorldToScreen(float worldX, float worldY, float elevation, int& screenX, int& screenY, const Camera& camera, const Player& player);
void RenderTile(SDL_Renderer* renderer, const Tile& tile, const Camera& camera, const Player& player);
void RenderPlayer(SDL_Renderer* renderer, const Player& player, const Camera& camera);

#endif // RENDERER_H
