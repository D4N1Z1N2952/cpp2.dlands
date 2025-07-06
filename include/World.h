#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <map>
#include "DataTypes.h" // For Tile, BiomeType, BiomeProperties, TerrainData
#include "Noise.h"     // For LayeredNoise, PerlinNoise (if directly used by world gen, though it seems LayeredNoise is the main interface)

// Function declarations for world generation and properties
std::vector<std::vector<Tile>> GenerateWorld();
BiomeType DetermineBiome(float elevation, float moisture); // Used by GenerateWorld
std::map<BiomeType, BiomeProperties> CreateBiomeProperties(); // Used by GenerateWorld
float GetTerrainHeight(float x, float y); // May or may not be used by GenerateWorld directly, but is world related

#endif // WORLD_H
