#include "utils/MathUtils.hpp"

#include <limits>
#include <random>

std::random_device rand_dev;
std::mt19937 generator(rand_dev());
std::uniform_real_distribution<f32> distr(0.0f, 1.0f);

u32 pcg_hash(u32 seed) {
    u32 state = seed * 747796405u + 2891336453u;
    u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

// generate float between 0 and 1
float rand_float(u32& seed) {
    return distr(generator);
    seed = pcg_hash(seed);
    return static_cast<float>(seed) / static_cast<float>(std::numeric_limits<u32>::max());
}
