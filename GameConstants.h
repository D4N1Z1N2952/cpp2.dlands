#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H

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

#endif // GAMECONSTANTS_H
