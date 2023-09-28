#include "utils/MathUtils.hpp"
#include <limits>


u32 pcg_hash(u32 seed) {
    u32 state = seed * 747796405u + 2891336453u;
    u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


float rand_float(u32& seed) {
    seed = pcg_hash(seed);
    return static_cast<float>(seed) / static_cast<float>(std::numeric_limits<u32>::max());
}