#pragma once
#include <numbers>
#include "types.hpp"

template<typename T>
static inline constexpr float to_radians(T degrees) {
    return degrees * (static_cast<T>(std::numbers::pi) / 180);
}


u32 pcg_hash(u32 seed);
float rand_float(u32& seed);


