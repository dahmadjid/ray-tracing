#include "utils/MathUtils.hpp"
#include <limits>


uint32_t pcg_hash(uint32_t seed) {
    uint32_t state = seed * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


float rand_float(uint32_t& seed) {
    seed = pcg_hash(seed);
    return static_cast<float>(seed) / static_cast<float>(std::numeric_limits<uint32_t>::max());
}