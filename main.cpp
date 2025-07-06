#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <map>

// Update these constants for better camera control
const float CAMERA_FOLLOW_SPEED = 0.1f; // Adjust this value between 0.05f and 0.2f for smooth following
const float PLAYER_SPEED = 5.0f; // Increased for better responsiveness
const float JUMP_FORCE = 10.0f; // Increased for higher jumps
const float GRAVITY = 0.3f;
const int PLAYER_SIZE = 30;
const float INITIAL_ELEVATION = 0.0f;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int TILE_WIDTH = 96;
const int TILE_HEIGHT = 48;
const int TILE_DEPTH = 24;
const int WORLD_WIDTH = 128;
const int WORLD_HEIGHT = 128;

// Camera
float cameraX = 0;
float cameraY = 0;

// Player constants
const int PLAYER_WIDTH = 20;
const int PLAYER_HEIGHT = 40;
const int HEAD_SIZE = 15;
const int LIMB_WIDTH = 6;
const int LIMB_LENGTH = 15;

// Add these new constants after the existing ones
const int NUM_OCTAVES = 4;
const float PERSISTENCE = 0.5f;
const float BASE_FREQUENCY = 0.01f;

// Add these new biome and terrain constants
const float WATER_LEVEL = 20.0f;
const float BEACH_LEVEL = 23.0f;
const float PLAINS_LEVEL = 35.0f;
const float HILLS_LEVEL = 50.0f;
const float MOUNTAIN_LEVEL = 70.0f;

// Add new noise layer constants
const int CONTINENT_OCTAVES = 4;
const int TERRAIN_OCTAVES = 6;
const int RIVER_OCTAVES = 2;
const float RIVER_THRESHOLD = 0.82f;

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
    
    void update(const Player& player) {
        // Convert player world position to screen space without camera offset
        float screenX = (player.x - player.y) * (TILE_WIDTH / 2);
        float screenY = (player.x + player.y) * (TILE_HEIGHT / 2) - player.elevation;

        // Immediately set camera to center player (no smoothing)
        // This ensures player is always perfectly centered
        x = screenX - (SCREEN_WIDTH / 2);
        y = screenY - (SCREEN_HEIGHT / 2);

        // Debug - but print less frequently to avoid console spam
        static Uint32 lastCameraDebugTime = 0;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastCameraDebugTime > 1000) { // Once per second
            std::cout << "Camera position: (" << x << ", " << y << ")" << std::endl;
            lastCameraDebugTime = currentTime;
        }
    }
};

// Function declarations
float GetTerrainHeight(float x, float y);
float PerlinNoise(float x, float y, int seed);
float LayeredNoise(float x, float y, int octaves, float persistence, float scale, int seed);
BiomeType DetermineBiome(float elevation, float moisture);
std::map<BiomeType, BiomeProperties> CreateBiomeProperties();
void WorldToScreen(float worldX, float worldY, float elevation, int& screenX, int& screenY, const Camera& camera, const Player& player) {
    // Player is always at center, so calculate positions relative to the player
    // Convert isometric coordinates relative to player's position
    float relativeX = worldX - player.x;
    float relativeY = worldY - player.y;
    float relativeElevation = elevation - player.elevation;

    // Convert to screen coordinates
    screenX = static_cast<int>((relativeX - relativeY) * (TILE_WIDTH / 2));
    screenY = static_cast<int>((relativeX + relativeY) * (TILE_HEIGHT / 2) - relativeElevation);

    // Add screen center offset to place player at center
    screenX += SCREEN_WIDTH / 2;
    screenY += SCREEN_HEIGHT / 2;
}

void HandlePlayerMovement(Player& player, const Uint8* keystate, float deltaTime, const std::vector<std::vector<Tile>>& world);
void RenderPlayer(SDL_Renderer* renderer, const Player& player, const Camera& camera) {
    // Player should always be at the center of the screen now
    int screenX = SCREEN_WIDTH / 2;
    int screenY = SCREEN_HEIGHT / 2;

    // Draw player body (rectangle)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Bright red color
    SDL_Rect playerRect = {
        screenX - PLAYER_WIDTH/2,
        screenY - PLAYER_HEIGHT,
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
        screenY - PLAYER_HEIGHT - HEAD_SIZE,
        HEAD_SIZE,
        HEAD_SIZE
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &head);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &head);

    // Add simple arms and legs for better visualization
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);

    // Arms
    SDL_RenderDrawLine(renderer, 
        screenX - PLAYER_WIDTH/2, screenY - PLAYER_HEIGHT + 10,
        screenX - PLAYER_WIDTH/2 - 10, screenY - PLAYER_HEIGHT + 20);
    SDL_RenderDrawLine(renderer, 
        screenX + PLAYER_WIDTH/2, screenY - PLAYER_HEIGHT + 10,
        screenX + PLAYER_WIDTH/2 + 10, screenY - PLAYER_HEIGHT + 20);

    // Legs
    SDL_RenderDrawLine(renderer, 
        screenX - PLAYER_WIDTH/4, screenY,
        screenX - PLAYER_WIDTH/4 - 5, screenY + 15);
    SDL_RenderDrawLine(renderer, 
        screenX + PLAYER_WIDTH/4, screenY,
        screenX + PLAYER_WIDTH/4 + 5, screenY + 15);
}
void UpdateGame(Player& player);
void ProcessInput(const Uint8* keyState, Player& player, const std::vector<std::vector<Tile>>& world);
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

        // Draw as filled polygon by creating triangles
        for (int i = 1; i < 3; i++) {
            SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[i].x, points[i].y);
            SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[i+1].x, points[i+1].y);
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        }

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
// Forward declaration of world generation function
std::vector<std::vector<Tile>> GenerateWorld();

// Add these helper functions first
float Fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float Grad(int hash, float x, float y) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Replace the incorrect PerlinNoise function with this corrected version
float PerlinNoise(float x, float y, int seed) {
    static const int permutation[256] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    // Create permutation table based on seed
    std::vector<int> p(512);
    std::mt19937 gen(seed);
    std::vector<int> perm(permutation, permutation + 256);
    std::shuffle(perm.begin(), perm.end(), gen);
    
    for(int i = 0; i < 256; i++) {
        p[i] = perm[i];
        p[256 + i] = perm[i];
    }

    // Calculate unit square coordinates
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    // Get relative coordinates within unit square
    x -= std::floor(x);
    y -= std::floor(y);

    // Compute fade curves
    float u = Fade(x);
    float v = Fade(y);

    // Hash coordinates of cube corners
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Add blended results from corners
    return Lerp(
        Lerp(
            Grad(p[AA], x, y),
            Grad(p[BA], x - 1, y),
            u
        ),
        Lerp(
            Grad(p[AB], x, y - 1),
            Grad(p[BB], x - 1, y - 1),
            u
        ),
        v
    );
}

// Simplified terrain height function for debugging visibility
float GetTerrainHeight(float x, float y) {
    // Simple height function that creates a gradual slope from corner to corner
    float normalizedX = x / WORLD_WIDTH;
    float normalizedY = y / WORLD_HEIGHT;

    // Create a simple height pattern for testing visibility
    float height = (normalizedX + normalizedY) * 50.0f;

    // Add some hills and variations
    height += 10.0f * sin(normalizedX * 10) * cos(normalizedY * 10);

    // Add a central mountain
    float distFromCenter = std::sqrt(
        std::pow(normalizedX - 0.5f, 2) + 
        std::pow(normalizedY - 0.5f, 2));

    height += 40.0f * std::max(0.0f, 1.0f - distFromCenter * 4.0f);

    return height;
}

// Add new noise function for different terrain features
float LayeredNoise(float x, float y, int octaves, float persistence, float scale, int seed) {
    float amplitude = 1.0f;
    float frequency = scale;
    float total = 0.0f;
    float maxValue = 0.0f;
    
    for(int i = 0; i < octaves; i++) {
        total += PerlinNoise(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

// Add new function to determine biome based on elevation and moisture
BiomeType DetermineBiome(float elevation, float moisture) {
    // Water biomes based on depth
    if (elevation < WATER_LEVEL - 5.0f) return BiomeType::DEEP_WATER;
    if (elevation < WATER_LEVEL) return BiomeType::SHALLOW_WATER;

    // Beach and coastal areas
    if (elevation < BEACH_LEVEL) return BiomeType::BEACH;

    // Lowlands - plains and forests based on moisture
    if (elevation < PLAINS_LEVEL) {
        if (moisture < 0.3f) return BiomeType::PLAINS; // Dry plains
        if (moisture < 0.6f) return BiomeType::PLAINS; // Standard plains
        return BiomeType::FOREST; // Forests in moist areas
    }

    // Hills and highlands
    if (elevation < HILLS_LEVEL) {
        if (moisture < 0.4f) return BiomeType::HILLS; // Dry hills
        return BiomeType::FOREST; // Forested hills
    }

    // Mountains and peaks
    if (elevation < MOUNTAIN_LEVEL) {
        return BiomeType::MOUNTAINS; // Rocky mountains
    }

    // Snow-capped peaks at highest elevations
    return BiomeType::SNOW_CAPS;
}

// Add this function after the BiomeProperties struct and BiomeType enum definitions
// but before any other functions that use it
std::map<BiomeType, BiomeProperties> CreateBiomeProperties() {
    return {
        {BiomeType::DEEP_WATER,    {{0, 64, 220, 255},    0.3f, 0.1f, false}},
        {BiomeType::SHALLOW_WATER, {{0, 128, 255, 255},   0.5f, 0.2f, false}},
        {BiomeType::BEACH,         {{240, 220, 180, 255}, 0.6f, 0.2f, true}},
        {BiomeType::PLAINS,        {{100, 210, 100, 255}, 1.0f, 0.3f, true}},
        {BiomeType::FOREST,        {{21, 120, 35, 255},   1.1f, 0.4f, true}},
        {BiomeType::HILLS,         {{90, 160, 90, 255},   1.2f, 0.6f, true}},
        {BiomeType::MOUNTAINS,     {{150, 140, 130, 255}, 1.5f, 0.8f, false}},
        {BiomeType::SNOW_CAPS,     {{255, 255, 255, 255}, 1.6f, 0.9f, false}}
    };
}

// Then modify the GenerateWorld function to use the map:
std::vector<std::vector<Tile>> GenerateWorld() {
    std::vector<std::vector<Tile>> world(WORLD_HEIGHT, std::vector<Tile>(WORLD_WIDTH));
    std::vector<std::vector<TerrainData>> terrainData(WORLD_HEIGHT, 
        std::vector<TerrainData>(WORLD_WIDTH));
    
    // Create the biome properties map
    auto biomeProps = CreateBiomeProperties();
    
    // Generate base terrain data
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            float nx = x / float(WORLD_WIDTH);
            float ny = y / float(WORLD_HEIGHT);

            // Generate continent shape - use a larger scale for bigger features
            float continentShape = LayeredNoise(nx * 0.5f, ny * 0.5f, CONTINENT_OCTAVES, 0.6f, 0.5f, 1);

            // Generate detailed terrain
            float terrainDetail = LayeredNoise(nx * 5.0f, ny * 5.0f, TERRAIN_OCTAVES, 0.5f, 2.0f, 2);

            // Generate mountain ranges (ridged noise)
            float mountain = 1.0f - std::abs(LayeredNoise(nx * 3.0f, ny * 3.0f, 4, 0.7f, 1.5f, 5) * 2.0f - 1.0f);
            mountain = std::pow(mountain, 3.0f); // Make mountains more pronounced

            // Generate moisture map
            float moisture = LayeredNoise(nx * 4.0f, ny * 4.0f, 4, 0.5f, 2.0f, 3);

            // Generate river map - use smaller scale noise for more varied rivers
            float riverValue = LayeredNoise(nx * 8.0f, ny * 8.0f, RIVER_OCTAVES, 0.7f, 3.0f, 4);

            // Distance from center for island-like generation
            float dx = nx - 0.5f;
            float dy = ny - 0.5f;
            float distanceFromCenter = std::sqrt(dx * dx + dy * dy) * 2.0f;
            float islandFactor = 1.0f - std::min(1.0f, distanceFromCenter);
            islandFactor = std::pow(islandFactor, 0.5f); // Adjust island shape

            // Combine elevation components
            float elevation = (continentShape * 0.5f + terrainDetail * 0.2f + mountain * 0.3f) * 100.0f;

            // Apply island shape (higher in center, lower at edges)
            elevation = elevation * (islandFactor * 0.7f + 0.3f);

            // Store terrain data
            terrainData[y][x].elevation = elevation;
            terrainData[y][x].moisture = moisture;
            terrainData[y][x].riverValue = riverValue;
        }
    }

    // Process rivers and create river networks
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            TerrainData& data = terrainData[y][x];

            // Create main rivers where river noise is above threshold
            if (data.riverValue > RIVER_THRESHOLD) {
                // Make rivers wider with a gradient
                float riverStrength = (data.riverValue - RIVER_THRESHOLD) / (1.0f - RIVER_THRESHOLD);

                // Apply river carving - lower elevation more for stronger river values
                data.elevation = std::min(data.elevation, WATER_LEVEL - riverStrength * 5.0f);

                // Create tributaries with lower threshold
                if (data.riverValue > RIVER_THRESHOLD - 0.1f && data.riverValue <= RIVER_THRESHOLD) {
                    data.elevation = std::min(data.elevation, WATER_LEVEL - 1.0f);
                }
            }

            // Create lakes in low areas with high moisture
            if (data.elevation < WATER_LEVEL + 5.0f && data.moisture > 0.7f) {
                data.elevation = std::min(data.elevation, WATER_LEVEL - 2.0f);
            }
        }
    }

    // Smooth terrain to avoid sharp transitions
    std::vector<std::vector<float>> smoothedElevation(WORLD_HEIGHT, std::vector<float>(WORLD_WIDTH));

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            // Apply simple 3x3 smoothing kernel
            float total = 0.0f;
            int count = 0;

            for (int ny = std::max(0, y-1); ny <= std::min(WORLD_HEIGHT-1, y+1); ++ny) {
                for (int nx = std::max(0, x-1); nx <= std::min(WORLD_WIDTH-1, x+1); ++nx) {
                    total += terrainData[ny][nx].elevation;
                    count++;
                }
            }

            // Don't smooth water areas too much to preserve river shapes
            if (terrainData[y][x].elevation <= WATER_LEVEL) {
                smoothedElevation[y][x] = terrainData[y][x].elevation;
            } else {
                smoothedElevation[y][x] = (total / count) * 0.7f + terrainData[y][x].elevation * 0.3f;
            }
        }
    }

    // Apply smoothed elevation back to terrain data
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            terrainData[y][x].elevation = smoothedElevation[y][x];
        }
    }

    // Finalize world and apply biomes
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            TerrainData& data = terrainData[y][x];

            // Determine biome based on elevation and moisture
            data.biome = DetermineBiome(data.elevation, data.moisture);

            // Apply terrain to world tiles
            Tile& tile = world[y][x];
            tile.x = x;
            tile.y = y;
            tile.elevation = static_cast<int>(data.elevation);

            // Get biome properties
            const BiomeProperties& props = biomeProps[data.biome];
            tile.walkable = props.walkable;

            // Use a simple color pattern for debugging visibility
            // Set color based on position in the grid for clear visibility
            if ((tile.x + tile.y) % 10 == 0) {
                // Grid pattern with bright red squares for reference points
                tile.color = {255, 0, 0, 255}; // Bright red
            } else if (tile.x == 0 || tile.y == 0 || tile.x == WORLD_WIDTH-1 || tile.y == WORLD_HEIGHT-1) {
                // Highlight world boundaries
                tile.color = {255, 255, 0, 255}; // Yellow
            } else if (tile.x == WORLD_WIDTH/2 || tile.y == WORLD_HEIGHT/2) {
                // Highlight center lines
                tile.color = {0, 0, 255, 255}; // Blue
            } else {
                // Normal biome-based coloring with high contrast
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> variation(-5, 5);

                tile.color = {
                    static_cast<Uint8>(std::clamp(props.baseColor.r + variation(gen), 0, 255)),
                    static_cast<Uint8>(std::clamp(props.baseColor.g + variation(gen), 0, 255)),
                    static_cast<Uint8>(std::clamp(props.baseColor.b + variation(gen), 0, 255)),
                    255
                };
            }
        }
    }
    
    return world;
}

// Add these implementations before the main() function

// Camera update is already handled by the Camera struct

// WorldToScreen function is already defined above with Camera parameter

// RenderTile function is already defined above with Camera parameter

// RenderPlayer function is already defined elsewhere

// RenderPlayer function implementation is already defined above

// Update HandlePlayerMovement for better control
void HandlePlayerMovement(Player& player, const Uint8* keystate, float deltaTime, const std::vector<std::vector<Tile>>& world) {
    // Increase speed for better response
    float moveSpeed = PLAYER_SPEED * deltaTime * 5.0f;

    // Store previous position
    float prevX = player.x;
    float prevY = player.y;

    // Print key states for debugging
    static Uint32 lastKeyPrintTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastKeyPrintTime > 500) {
        std::cout << "Key states: W:" << (keystate[SDL_SCANCODE_W] ? "1" : "0")
                  << " A:" << (keystate[SDL_SCANCODE_A] ? "1" : "0")
                  << " S:" << (keystate[SDL_SCANCODE_S] ? "1" : "0")
                  << " D:" << (keystate[SDL_SCANCODE_D] ? "1" : "0") << std::endl;
        lastKeyPrintTime = currentTime;
    }

    // Handle movement - isometric style with corrected directions
    // In isometric view, moving "forward" means decreasing Y
    if (keystate[SDL_SCANCODE_W]) {
        // Move forward (north)
        player.y -= moveSpeed;
        std::cout << "Moving FORWARD" << std::endl;
    }
    if (keystate[SDL_SCANCODE_S]) {
        // Move backward (south)
        player.y += moveSpeed;
        std::cout << "Moving BACKWARD" << std::endl;
    }
    if (keystate[SDL_SCANCODE_A]) {
        // Move left (west)
        player.x -= moveSpeed;
        std::cout << "Moving LEFT" << std::endl;
    }
    if (keystate[SDL_SCANCODE_D]) {
        // Move right (east)
        player.x += moveSpeed;
        std::cout << "Moving RIGHT" << std::endl;
    }

    // Keep player within world bounds with some margin
    player.x = std::max(1.0f, std::min(player.x, float(WORLD_WIDTH - 2)));
    player.y = std::max(1.0f, std::min(player.y, float(WORLD_HEIGHT - 2)));

    // Check if new position is walkable
    int tileX = static_cast<int>(player.x);
    int tileY = static_cast<int>(player.y);

    if (tileX >= 0 && tileX < WORLD_WIDTH && tileY >= 0 && tileY < WORLD_HEIGHT) {
        // Get tile at player position
        const Tile& currentTile = world[tileY][tileX];

        // Always make tiles walkable for now to avoid movement issues
        // If tile is not walkable, revert to previous position
        if (false && !currentTile.walkable) { // Disabled for debugging
            player.x = prevX;
            player.y = prevY;
            std::cout << "Blocked by non-walkable tile" << std::endl;
        } else {
            // Adjust player elevation to match the terrain
            // Smoothly interpolate to terrain height if not jumping
            if (!player.isJumping) {
                float targetElevation = static_cast<float>(currentTile.elevation);
                player.elevation += (targetElevation - player.elevation) * 0.2f;
            }
        }
    }

    // Handle jumping
    if (keystate[SDL_SCANCODE_SPACE] && !player.isJumping) {
        player.velocityZ = JUMP_FORCE;
        player.isJumping = true;
        std::cout << "Jumping!" << std::endl;
    }

    // Apply gravity and update elevation
    if (player.isJumping) {
        player.velocityZ -= GRAVITY;
        player.elevation += player.velocityZ;

        // Get the terrain height at player position
        int tileX = static_cast<int>(player.x);
        int tileY = static_cast<int>(player.y);
        float terrainHeight = 0.0f;

        if (tileX >= 0 && tileX < WORLD_WIDTH && tileY >= 0 && tileY < WORLD_HEIGHT) {
            terrainHeight = static_cast<float>(world[tileY][tileX].elevation);
        }

        // Check for landing
        if (player.elevation <= terrainHeight) {
            player.elevation = terrainHeight;
            player.velocityZ = 0;
            player.isJumping = false;
            std::cout << "Landed" << std::endl;
        }
    }

    // Print player position when it changes
    static float lastX = 0, lastY = 0;
    if (std::abs(player.x - lastX) > 0.1f || std::abs(player.y - lastY) > 0.1f) {
        std::cout << "Player moved to: (" << player.x << ", " << player.y << ", " << player.elevation << ")" << std::endl;
        lastX = player.x;
        lastY = player.y;
    }
}

// Biome properties are defined in CreateBiomeProperties function

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