#ifndef NOISE_H
#define NOISE_H

#include <vector>
#include <cmath>
#include <random>
#include <algorithm> // Required for std::shuffle

// Function declarations for noise generation
float PerlinNoise(float x, float y, int seed);
float LayeredNoise(float x, float y, int octaves, float persistence, float scale, int seed);

// Helper functions for Perlin Noise (often kept in the .cpp or a detail namespace)
float Fade(float t);
float Grad(int hash, float x, float y); // Note: y parameter was missing in main.cpp's declaration, but present in definition
float Lerp(float a, float b, float t);

#endif // NOISE_H
