#include "Noise.h"
#include <vector> // Already included via Noise.h but good for clarity
#include <cmath>  // Already included via Noise.h
#include <numeric> // For std::iota if needed, though not used in current Perlin
#include <algorithm> // For std::shuffle, already included via Noise.h
#include <random>    // For std::mt19937, already included via Noise.h

// Helper functions (definitions)
float Fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float Grad(int hash, float x, float y) { // Added y parameter to match definition style
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0); // Corrected logic as per typical Perlin grad functions
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// PerlinNoise function definition
float PerlinNoise(float x, float y, int seed) {
    // Static permutation table, ensures it's initialized once.
    // Using a fixed one for now, can be seeded/shuffled if more variation is needed across runs with same seed.
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

    // Create a permutation table p, possibly shuffled by seed.
    // For simplicity and to match the original code's use of a single shuffled perm table,
    // we'll make p static as well, and shuffle it once per seed if the seed changes.
    // A more robust way would be to have a NoiseGenerator class that holds its seeded p table.
    static std::vector<int> p(512);
    static int lastSeed = -1; // Ensure it's different from any initial seed.

    if (seed != lastSeed) {
        std::vector<int> base_perm(permutation, permutation + 256);
        std::mt19937 gen(seed);
        std::shuffle(base_perm.begin(), base_perm.end(), gen);

        for(int i = 0; i < 256; i++) {
            p[i] = base_perm[i];
            p[256 + i] = base_perm[i];
        }
        lastSeed = seed;
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

// LayeredNoise function definition
float LayeredNoise(float x, float y, int octaves, float persistence, float scale, int seed) {
    float amplitude = 1.0f;
    float frequency = scale;
    float total = 0.0f;
    float maxValue = 0.0f; // Used for normalization

    for(int i = 0; i < octaves; i++) {
        total += PerlinNoise(x * frequency, y * frequency, seed + i) * amplitude; // Use different seed for each octave
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    // Normalize the result to be between -1 and 1 (approximately, Perlin can exceed this slightly)
    // Or, if only positive values are desired, map it to [0, 1]
    return maxValue > 0 ? total / maxValue : 0; // Avoid division by zero
}
