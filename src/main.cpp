#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <map>

#include "../include/GameConstants.h"
#include "../include/Player.h"
#include "../include/Camera.h"
#include "../include/DataTypes.h"
#include "../include/Noise.h"
#include "../include/World.h"
#include "../include/Renderer.h"

// Camera global variables - these might be better inside the Camera struct or a GameState class later
float cameraX_global = 0; // Renamed to avoid conflict if Camera struct members are named x,y
float cameraY_global = 0;

// Update main game loop
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "2.5D Lands - Debug Mode",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Generating world..." << std::endl;
    std::cout << "World size: " << WORLD_WIDTH << "x" << WORLD_HEIGHT << std::endl;
    std::cout << "Screen size: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "Tile dimensions: " << TILE_WIDTH << "x" << TILE_HEIGHT << " (depth: " << TILE_DEPTH << ")" << std::endl;

    // Generate a smaller world for debugging if needed
    auto world = GenerateWorld();

    // Output terrain statistics
    int waterTiles = 0, landTiles = 0, mountainTiles = 0;
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            if (world[y][x].elevation < WATER_LEVEL) waterTiles++;
            else if (world[y][x].elevation > MOUNTAIN_LEVEL) mountainTiles++;
            else landTiles++;
        }
    }

    std::cout << "World generation complete!" << std::endl;
    std::cout << "Water tiles: " << waterTiles << " (" << (100.0f * waterTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;
    std::cout << "Land tiles: " << landTiles << " (" << (100.0f * landTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;
    std::cout << "Mountain tiles: " << mountainTiles << " (" << (100.0f * mountainTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;

    // Simple player initialization at a fixed position for debugging
    Player player;
    player.x = 10.0f;
    player.y = 10.0f;
    player.elevation = 30.0f; // Start at a visible elevation

    // Initialize camera with explicit position
    Camera camera;
    camera.x = 0;
    camera.y = 0;

    // Print initial positions
    std::cout << "Initial player position: (" << player.x << ", " << player.y << ", " << player.elevation << ")" << std::endl;

    Uint32 lastTime = SDL_GetTicks();
    bool running = true;
    SDL_Event event;

    // Display help message
    std::cout << "Controls:\n"
              << "W - Move forward (north)\n"
              << "S - Move backward (south)\n"
              << "A - Move left (west)\n"
              << "D - Move right (east)\n"
              << "Arrow Keys - Alternative movement\n"
              << "SPACE - Jump\n"
              << "F3 - Toggle debug mode\n"
              << "R - Reset player position\n"
              << "T - Test tile rendering\n"
              << "ESC - Quit game\n" << std::endl;

    bool debugMode = true; // Start with debug mode enabled for visibility

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                else if (event.key.keysym.sym == SDLK_F3) {
                    debugMode = !debugMode;
                    std::cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << std::endl;
                }
                // Alternative movement with arrow keys for testing
                else if (event.key.keysym.sym == SDLK_UP) {
                    player.y -= 1.0f;
                    std::cout << "Arrow UP pressed: Player at (" << player.x << ", " << player.y << ")" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    player.y += 1.0f;
                    std::cout << "Arrow DOWN pressed: Player at (" << player.x << ", " << player.y << ")" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    player.x -= 1.0f;
                    std::cout << "Arrow LEFT pressed: Player at (" << player.x << ", " << player.y << ")" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    player.x += 1.0f;
                    std::cout << "Arrow RIGHT pressed: Player at (" << player.x << ", " << player.y << ")" << std::endl;
                }
                // Reset player position
                else if (event.key.keysym.sym == SDLK_r) {
                    player.x = 10.0f;
                    player.y = 10.0f;
                    player.elevation = 30.0f;
                    std::cout << "Player position reset to (10, 10, 30)" << std::endl;
                }
                // Testing key for direct tile rendering
                else if (event.key.keysym.sym == SDLK_t) {
                    // Draw a test tile in the center of the screen
                    Tile testTile;
                    testTile.x = 0;
                    testTile.y = 0;
                    testTile.elevation = 0;
                    testTile.color = {255, 0, 0, 255};

                    Camera testCam;
                    testCam.x = 0;
                    testCam.y = 0;

                    std::cout << "Drawing test tile at center" << std::endl;
                    RenderTile(renderer, testTile, testCam, player);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(1000); // Pause to see the test tile
                }
            }
        }

        // Get current keyboard state for continuous movement
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);

        // Process player movement with debugging output
        static Uint32 lastMovementTime = 0;
        if (currentTime - lastMovementTime > 16) { // Cap at ~60 fps for movement
            HandlePlayerMovement(player, keyState, deltaTime, world);
            lastMovementTime = currentTime;
        }

        // Update camera to follow player
        camera.update(player);

        SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255); // Dark blue-gray background
        SDL_RenderClear(renderer);

        // Print camera and player position for debugging (only once per second)
        static Uint32 lastDebugTime = 0;
        if (currentTime - lastDebugTime > 1000) {
            std::cout << "Camera: (" << camera.x << ", " << camera.y << ")" << std::endl;
            std::cout << "Player: (" << player.x << ", " << player.y << ", " << player.elevation << ")" << std::endl;
            lastDebugTime = currentTime;
        }

        // Simpler rendering approach to debug visibility
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int x = 0; x < WORLD_WIDTH; x++) {
                // Only render tiles that are likely to be visible
                int screenX, screenY;
                WorldToScreen(x, y, world[y][x].elevation, screenX, screenY, camera, player);

                // Check if the tile is potentially visible on screen (with margin)
                if (screenX > -TILE_WIDTH && screenX < SCREEN_WIDTH + TILE_WIDTH &&
                    screenY > -TILE_HEIGHT && screenY < SCREEN_HEIGHT + TILE_HEIGHT) {
                    RenderTile(renderer, world[y][x], camera, player);
                }
            }
        }

        RenderPlayer(renderer, player, camera);

        // Draw debug information on screen
        if (debugMode) {
            // Draw coordinate grid
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);

            // Draw horizontal grid lines
            for (int i = 0; i <= 10; i++) {
                int y = i * SCREEN_HEIGHT / 10;
                SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
            }

            // Draw vertical grid lines
            for (int i = 0; i <= 10; i++) {
                int x = i * SCREEN_WIDTH / 10;
                SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
            }

            // Draw screen center crosshair
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2);
            SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 20, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 20);

            // Display player coordinates as text
            // Since SDL doesn't have built-in text rendering, we'll create a visual indicator
            // Draw coordinate box in the top-left corner
            SDL_Rect coordBox = {10, 10, 200, 60};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &coordBox);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &coordBox);

            // Draw coordinate lines to represent values
            // X coordinate - Red line
            int xLineLength = static_cast<int>(player.x * 2);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(renderer, 20, 25, 20 + xLineLength, 25);

            // Y coordinate - Green line
            int yLineLength = static_cast<int>(player.y * 2);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(renderer, 20, 45, 20 + yLineLength, 45);

            // Z/Elevation coordinate - Blue line
            int zLineLength = static_cast<int>(player.elevation * 2);
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawLine(renderer, 20, 65, 20 + zLineLength, 65);

            // Also update window title with coordinates
            char title[100];
            snprintf(title, sizeof(title), "2.5D Lands - Player: X=%.1f, Y=%.1f, Z=%.1f", player.x, player.y, player.elevation);
            SDL_SetWindowTitle(window, title);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}