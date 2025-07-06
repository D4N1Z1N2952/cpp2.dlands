#include "Renderer.h"
#include <cmath>      // For std::abs (though not used in current RenderTile, good for graphics)
#include <iostream>   // For debug output (e.g. in RenderTile, can be removed)

// Converts world coordinates to screen coordinates
void WorldToScreen(float worldX, float worldY, float elevation, int& screenX, int& screenY, const Camera& camera, const Player& player) {
    // Player is always at center, so calculate positions relative to the player
    // Convert isometric coordinates relative to player's position
    float relativeX = worldX - player.x;
    float relativeY = worldY - player.y;
    float relativeElevation = elevation - player.elevation;

    // Convert to screen coordinates
    screenX = static_cast<int>((relativeX - relativeY) * (TILE_WIDTH / 2.0f)); // Use floating point division
    screenY = static_cast<int>((relativeX + relativeY) * (TILE_HEIGHT / 2.0f) - relativeElevation); // Use floating point division

    // Add screen center offset to place player at center
    screenX += SCREEN_WIDTH / 2;
    screenY += SCREEN_HEIGHT / 2;
}

// Renders a single tile
void RenderTile(SDL_Renderer* renderer, const Tile& tile, const Camera& camera, const Player& player) {
    int screenX, screenY;
    WorldToScreen(tile.x, tile.y, tile.elevation, screenX, screenY, camera, player);

    // Calculate tile corners in screen space
    SDL_Point points[5] = {
        {screenX, screenY - (TILE_HEIGHT / 2)},                    // Top
        {screenX + (TILE_WIDTH / 2), screenY},                     // Right
        {screenX, screenY + (TILE_HEIGHT / 2)},                    // Bottom
        {screenX - (TILE_WIDTH / 2), screenY},                     // Left
        {screenX, screenY - (TILE_HEIGHT / 2)}                     // Back to top
    };

    // Only render if tile is potentially visible (with margin)
    if (screenX + TILE_WIDTH > 0 && screenX - TILE_WIDTH < SCREEN_WIDTH &&
        screenY + TILE_HEIGHT > 0 && screenY - TILE_HEIGHT < SCREEN_HEIGHT) {

        // Draw filled tile for better visibility
        SDL_SetRenderDrawColor(renderer, tile.color.r, tile.color.g, tile.color.b, tile.color.a);

        // Draw as filled polygon by creating triangles (simple approach for isometric diamond)
        // This can be improved with SDL_RenderGeometry for more robust polygon filling if needed.
        // For a simple diamond, two triangles suffice: (Top, Right, Bottom) and (Top, Bottom, Left)
        SDL_Vertex vertices[4] = {
            { { (float)points[0].x, (float)points[0].y }, tile.color, { 0, 0 } }, // Top
            { { (float)points[1].x, (float)points[1].y }, tile.color, { 0, 0 } }, // Right
            { { (float)points[2].x, (float)points[2].y }, tile.color, { 0, 0 } }, // Bottom
            { { (float)points[3].x, (float)points[3].y }, tile.color, { 0, 0 } }  // Left
        };

        // Triangle 1: Top, Right, Bottom
        // Triangle 2: Top, Bottom, Left
        // Indices for SDL_RenderGeometry.
        // If not using SDL_RenderGeometry, this part needs manual triangle drawing calls or line drawing.
        // The original code used line drawing which doesn't actually fill.
        // Let's stick to the original line drawing for now to maintain behavior, then improve.
        // The original code's "filled polygon by creating triangles" was actually just drawing lines *between* points of triangles.
        // For true filling, SDL_gfx or SDL_RenderGeometry would be better.
        // Replicating original line drawing:
        // for (int i = 1; i < 3; i++) {
        //     SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[i].x, points[i].y);
        //     SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[i+1].x, points[i+1].y);
        //     SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        // }
        // A simpler way to draw a filled diamond shape with lines (though not truly filled):
        // Draw lines from center to each point, then connect points.
        // However, to match the original's attempt at filling with lines:
        SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[1].x, points[1].y); // Top to Right
        SDL_RenderDrawLine(renderer, points[1].x, points[1].y, points[2].x, points[2].y); // Right to Bottom
        SDL_RenderDrawLine(renderer, points[2].x, points[2].y, points[3].x, points[3].y); // Bottom to Left
        SDL_RenderDrawLine(renderer, points[3].x, points[3].y, points[0].x, points[0].y); // Left to Top
        // To make it look more "filled" with lines, also draw diagonals:
        SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[2].x, points[2].y); // Top to Bottom
        SDL_RenderDrawLine(renderer, points[1].x, points[1].y, points[3].x, points[3].y); // Right to Left


        // Draw outline for clarity
        SDL_SetRenderDrawColor(renderer,
            tile.color.r > 128 ? tile.color.r - 40 : tile.color.r + 40,
            tile.color.g > 128 ? tile.color.g - 40 : tile.color.g + 40,
            tile.color.b > 128 ? tile.color.b - 40 : tile.color.b + 40,
            255);

        for (int i = 0; i < 4; i++) {
            SDL_RenderDrawLine(renderer,
                points[i].x, points[i].y,
                points[i + 1].x, points[i + 1].y);
        }
    }
}

// Renders the player
void RenderPlayer(SDL_Renderer* renderer, const Player& player, const Camera& camera) {
    // Player should always be at the center of the screen now
    // The 'camera' parameter might seem redundant if player is always screen center,
    // but WorldToScreen uses it, and it's good practice for a RenderPlayer function.
    // For this specific implementation where player is screen center:
    int screenX = SCREEN_WIDTH / 2;
    int screenY = SCREEN_HEIGHT / 2;

    // Draw player body (rectangle)
    SDL_SetRenderDrawColor(renderer, player.color.r, player.color.g, player.color.b, player.color.a);
    SDL_Rect playerRect = {
        screenX - PLAYER_WIDTH/2,
        screenY - PLAYER_HEIGHT, // Player's y is at their feet, rect is drawn upwards
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };
    SDL_RenderFillRect(renderer, &playerRect);

    // Draw player outline
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black outline
    SDL_RenderDrawRect(renderer, &playerRect);

    // Draw head
    SDL_Rect head = {
        screenX - HEAD_SIZE/2,
        screenY - PLAYER_HEIGHT - HEAD_SIZE, // Head on top of body
        HEAD_SIZE,
        HEAD_SIZE
    };
    SDL_SetRenderDrawColor(renderer, player.color.r, player.color.g, player.color.b, player.color.a); // Player color for head
    SDL_RenderFillRect(renderer, &head);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black outline for head
    SDL_RenderDrawRect(renderer, &head);

    // Add simple arms and legs for better visualization
    // Using a slightly darker shade of player color for limbs
    SDL_SetRenderDrawColor(renderer,
        (Uint8)(player.color.r > 50 ? player.color.r - 50 : 0),
        (Uint8)(player.color.g > 50 ? player.color.g - 50 : 0),
        (Uint8)(player.color.b > 50 ? player.color.b - 50 : 0),
        player.color.a);

    // Arms
    SDL_RenderDrawLine(renderer,
        screenX - PLAYER_WIDTH/2, screenY - PLAYER_HEIGHT + 10, // From body side
        screenX - PLAYER_WIDTH/2 - LIMB_LENGTH, screenY - PLAYER_HEIGHT + 10 + LIMB_LENGTH/2); // Arm extending outwards
    SDL_RenderDrawLine(renderer,
        screenX + PLAYER_WIDTH/2, screenY - PLAYER_HEIGHT + 10, // From body side
        screenX + PLAYER_WIDTH/2 + LIMB_LENGTH, screenY - PLAYER_HEIGHT + 10 + LIMB_LENGTH/2); // Arm extending outwards

    // Legs
    SDL_RenderDrawLine(renderer,
        screenX - PLAYER_WIDTH/4, screenY, // From bottom-center of body
        screenX - PLAYER_WIDTH/4 - LIMB_LENGTH/2, screenY + LIMB_LENGTH); // Leg extending downwards/outwards
    SDL_RenderDrawLine(renderer,
        screenX + PLAYER_WIDTH/4, screenY, // From bottom-center of body
        screenX + PLAYER_WIDTH/4 + LIMB_LENGTH/2, screenY + LIMB_LENGTH); // Leg extending downwards/outwards
}
