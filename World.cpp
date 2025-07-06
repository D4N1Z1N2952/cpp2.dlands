#include "World.h"
#include "GameConstants.h" // For WORLD_WIDTH, WORLD_HEIGHT, biome levels etc.
#include <cmath>           // For std::sqrt, std::pow, std::abs, std::sin, std::cos, std::max, std::min
#include <random>          // For std::random_device, std::mt19937, std::uniform_int_distribution
#include <algorithm>       // For std::clamp, std::max, std::min (though cmath also has max/min)
#include <iostream>        // For std::cout in GenerateWorld (debug output, might remove later)

// Simplified terrain height function (can be expanded or made more complex later)
// This version was marked for debugging visibility in main.cpp
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

// Function to determine biome based on elevation and moisture
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

// Function to create biome properties map
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

// World generation function
std::vector<std::vector<Tile>> GenerateWorld() {
    std::vector<std::vector<Tile>> world(WORLD_HEIGHT, std::vector<Tile>(WORLD_WIDTH));
    std::vector<std::vector<TerrainData>> terrainData(WORLD_HEIGHT,
        std::vector<TerrainData>(WORLD_WIDTH));

    auto biomeProps = CreateBiomeProperties();

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            float nx = x / float(WORLD_WIDTH);
            float ny = y / float(WORLD_HEIGHT);

            float continentShape = LayeredNoise(nx * 0.5f, ny * 0.5f, CONTINENT_OCTAVES, 0.6f, 0.5f, 1);
            float terrainDetail = LayeredNoise(nx * 5.0f, ny * 5.0f, TERRAIN_OCTAVES, 0.5f, 2.0f, 2);
            float mountain = 1.0f - std::abs(LayeredNoise(nx * 3.0f, ny * 3.0f, 4, 0.7f, 1.5f, 5) * 2.0f - 1.0f);
            mountain = std::pow(mountain, 3.0f);
            float moisture = LayeredNoise(nx * 4.0f, ny * 4.0f, 4, 0.5f, 2.0f, 3);
            float riverValue = LayeredNoise(nx * 8.0f, ny * 8.0f, RIVER_OCTAVES, 0.7f, 3.0f, 4);

            float dx = nx - 0.5f;
            float dy = ny - 0.5f;
            float distanceFromCenter = std::sqrt(dx * dx + dy * dy) * 2.0f;
            float islandFactor = 1.0f - std::min(1.0f, distanceFromCenter);
            islandFactor = std::pow(islandFactor, 0.5f);

            float elevation = (continentShape * 0.5f + terrainDetail * 0.2f + mountain * 0.3f) * 100.0f;
            elevation = elevation * (islandFactor * 0.7f + 0.3f);

            terrainData[y][x].elevation = elevation;
            terrainData[y][x].moisture = moisture;
            terrainData[y][x].riverValue = riverValue;
        }
    }

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            TerrainData& data = terrainData[y][x];
            if (data.riverValue > RIVER_THRESHOLD) {
                float riverStrength = (data.riverValue - RIVER_THRESHOLD) / (1.0f - RIVER_THRESHOLD);
                data.elevation = std::min(data.elevation, WATER_LEVEL - riverStrength * 5.0f);
                if (data.riverValue > RIVER_THRESHOLD - 0.1f && data.riverValue <= RIVER_THRESHOLD) {
                    data.elevation = std::min(data.elevation, WATER_LEVEL - 1.0f);
                }
            }
            if (data.elevation < WATER_LEVEL + 5.0f && data.moisture > 0.7f) {
                data.elevation = std::min(data.elevation, WATER_LEVEL - 2.0f);
            }
        }
    }

    std::vector<std::vector<float>> smoothedElevation(WORLD_HEIGHT, std::vector<float>(WORLD_WIDTH));
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            float total = 0.0f;
            int count = 0;
            for (int ny = std::max(0, y-1); ny <= std::min(WORLD_HEIGHT-1, y+1); ++ny) {
                for (int nx = std::max(0, x-1); nx <= std::min(WORLD_WIDTH-1, x+1); ++nx) {
                    total += terrainData[ny][nx].elevation;
                    count++;
                }
            }
            if (terrainData[y][x].elevation <= WATER_LEVEL) {
                smoothedElevation[y][x] = terrainData[y][x].elevation;
            } else {
                smoothedElevation[y][x] = (total / count) * 0.7f + terrainData[y][x].elevation * 0.3f;
            }
        }
    }

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            terrainData[y][x].elevation = smoothedElevation[y][x];
        }
    }

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            TerrainData& data = terrainData[y][x];
            data.biome = DetermineBiome(data.elevation, data.moisture);
            Tile& tile = world[y][x];
            tile.x = x;
            tile.y = y;
            tile.elevation = static_cast<int>(data.elevation);
            const BiomeProperties& props = biomeProps.at(data.biome); // Use .at() for safety, or [] if sure key exists
            tile.walkable = props.walkable;

            // Debug coloring pattern from main.cpp
            if ((tile.x + tile.y) % 10 == 0) {
                tile.color = {255, 0, 0, 255};
            } else if (tile.x == 0 || tile.y == 0 || tile.x == WORLD_WIDTH-1 || tile.y == WORLD_HEIGHT-1) {
                tile.color = {255, 255, 0, 255};
            } else if (tile.x == WORLD_WIDTH/2 || tile.y == WORLD_HEIGHT/2) {
                tile.color = {0, 0, 255, 255};
            } else {
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

    // Debug output from main.cpp (can be removed or made conditional later)
    int waterTiles = 0, landTiles = 0, mountainTiles = 0;
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            if (world[y][x].elevation < WATER_LEVEL) waterTiles++;
            else if (world[y][x].elevation > MOUNTAIN_LEVEL) mountainTiles++;
            else landTiles++;
        }
    }
    std::cout << "World generation (from World.cpp) complete!" << std::endl;
    std::cout << "Water tiles: " << waterTiles << " (" << (100.0f * waterTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;
    std::cout << "Land tiles: " << landTiles << " (" << (100.0f * landTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;
    std::cout << "Mountain tiles: " << mountainTiles << " (" << (100.0f * mountainTiles / (WORLD_WIDTH * WORLD_HEIGHT)) << "%)" << std::endl;

    return world;
}
