#pragma once

#include <fmt/format.h>

#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>

#include "linear_algebra/Vec3.decl.hpp"
#include "utils/MathUtils.hpp"

template <typename T>
constexpr Vec3<T>::Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

template <typename T>
Vec3<T>::Vec3() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)) {}

template <typename T>
Vec3<T>::Vec3(T value) : x(value), y(value), z(value) {}

template <typename T>
T Vec3<T>::dot(const Vec3<T>& rhs) const {
    return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
}

template <typename T>
Vec3<T> Vec3<T>::cross(const Vec3& rhs) const {
    return Vec3(
        this->y * rhs.z - this->z * rhs.y, this->z * rhs.x - this->x * rhs.z, this->x * rhs.y - this->y * rhs.x
    );
}

template <typename T>
auto Vec3<T>::length_squared() const {
    return this->dot(*this);
}

template <typename T>
auto Vec3<T>::length() const {
    return std::sqrt(this->length_squared());
}

template <typename T>
Vec3<T>& Vec3<T>::scale(T factor) {
    this->x *= factor;
    this->y *= factor;
    this->z *= factor;
    return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::normalize() {
    T one_over_length = static_cast<T>(1) / this->length();
    this->x *= one_over_length;
    this->y *= one_over_length;
    this->z *= one_over_length;
    return *this;
}

template <typename T>
Vec3<uint32_t> Vec3<T>::floor() const {
    return Vec3<uint32_t>(
        static_cast<uint32_t>(std::floor(this->x)), static_cast<uint32_t>(std::floor(this->y)),
        static_cast<uint32_t>(std::floor(this->z))
    );
}

template <typename T>
Vec3<uint32_t> Vec3<T>::ceil() const {
    return Vec3<uint32_t>(
        static_cast<uint32_t>(std::ceil(this->x)), static_cast<uint32_t>(std::ceil(this->y)),
        static_cast<uint32_t>(std::ceil(this->z))
    );
}

template <typename T>
Vec3<T>& Vec3<T>::shift(T factor) {
    this->x += factor;
    this->y += factor;
    this->z += factor;
    return *this;
}

template <typename T>
Vec3<T> Vec3<T>::operator-() const {
    return Vec3(-this->x, -this->y, -this->z);
}

template <typename T>
Vec3<T>& Vec3<T>::rotate(const Quaternion<T>& rotation_quaternion) {
    auto quat_to_rotate = Quaternion<T>(static_cast<T>(0), this->x, this->y, this->z);
    auto result = rotation_quaternion.cross(quat_to_rotate).cross(-rotation_quaternion);
    this->x = result.x;
    this->y = result.y;
    this->z = result.z;
    return *this;
}

template <typename T>
struct fmt::formatter<Vec3<T>> : formatter<std::string> {
    auto format(const Vec3<T>& vec, fmt::format_context& ctx) const {
        std::string out = fmt::format("Vec3({}, {}, {})", vec.x, vec.y, vec.z);
        return formatter<std::string>::format(out, ctx);
    }
};

template <typename T>
Vec3<float> Vec3<T>::random(u32& seed) {
    return Vec3<float>(rand_float(seed) * 2 - 1, rand_float(seed) * 2 - 1, rand_float(seed) * 2 - 1);
}

template <typename T>
Vec3<T> Vec3<T>::reflect(const Vec3<T>& axis) const {
    return *this - Vec3(axis).scale(this->dot(axis) * static_cast<T>(2));
}

template <typename T>
void Vec3<T>::operator+=(const Vec3<T>& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
}

template <typename T>
void Vec3<T>::operator-=(const Vec3<T>& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
}

template <typename T>
void Vec3<T>::operator*=(const Vec3<T>& rhs) {
    this->x *= rhs.x;
    this->y *= rhs.y;
    this->z *= rhs.z;
}

template <typename T>
void Vec3<T>::operator/=(const Vec3<T>& rhs) {
    this->x /= rhs.x;
    this->y /= rhs.y;
    this->z /= rhs.z;
}

template <typename T>
template <typename U>
Vec3<U> Vec3<T>::cast() {
    return Vec3<U>(static_cast<U>(this->x), static_cast<U>(this->y), static_cast<U>(this->z));
}

template <typename T>
Vec3<T>& Vec3<T>::clamp(T min, T max) {
    this->x = std::max(std::min(this->x, max), min);
    this->y = std::max(std::min(this->y, max), min);
    this->z = std::max(std::min(this->z, max), min);
    return *this;
}

template <typename T>
template <typename IndexType>
T& Vec3<T>::operator[](IndexType i) {
    if (i == 0) {
        return this->x;
    }
    if (i == 1) {
        return this->y;
    }
    return this->z;
}

template <typename T>
template <typename IndexType>
T Vec3<T>::operator[](IndexType i) const {
    if (i == 0) {
        return this->x;
    }
    if (i == 1) {
        return this->y;
    }
    return this->z;
}

using Vec3f = Vec3<f32>;