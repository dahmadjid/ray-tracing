#pragma once
#include <numbers>

#include "types.hpp"

#define PI (f32) std::numbers::pi
#define INV_PI (f32) std::numbers::inv_pi

template <typename T>
inline constexpr float to_radians(T degrees) {
    return degrees * (static_cast<T>(std::numbers::pi) / 180);
}

template <typename T>
inline bool between(const T& x, const T& min, const T& max) {
    return x >= min && x <= max;
}

template <typename T>
inline T clamp(T n, T min, T max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

template <typename T>
inline T mix(T x, T y, T a) {
    return x * (1 - a) + y * a;
}

u32 pcg_hash(u32 seed);
float rand_float(u32& seed);