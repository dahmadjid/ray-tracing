#pragma once
#include <cstdint>
#include <numbers>

template<typename T>
static inline constexpr float to_radians(T degrees) {
    return degrees * (std::numbers::pi / 180);
}


uint32_t pcg_hash(uint32_t seed);
float rand_float(uint32_t& seed);
