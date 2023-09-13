#pragma once
#include <numbers>

template<typename T>
static inline constexpr float to_radians(T degrees) {
    return degrees * (std::numbers::pi / 180);
}