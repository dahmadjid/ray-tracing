#pragma once
#include <cstdint>
#include <numbers>

typedef __int8_t i8;
typedef __uint8_t u8;
typedef __int16_t i16;
typedef __uint16_t u16;
typedef __int32_t i32;
typedef __uint32_t u32;
typedef __int64_t i64;
typedef __uint64_t u64;
typedef float f32;
typedef double f64;


template<typename T>
static inline constexpr float to_radians(T degrees) {
    return degrees * (std::numbers::pi / 180);
}


u32 pcg_hash(u32 seed);
float rand_float(u32& seed);


