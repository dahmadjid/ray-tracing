#pragma once

#include <fmt/format.h>

#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <mutex>
#include "linear_algebra/Vec3.decl.hpp"
#include "utils/MathUtils.hpp"
#include <rfl/json.hpp>
using Vec3f = Vec3<f32>;

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

struct Vec3NormalizeInput {
    Vec3f vec;
};

struct Vec3NormalizeOutput {
    Vec3f vec_normalized;
};

struct Vec3NormalizeTestData {
    Vec3NormalizeInput input;
    Vec3NormalizeOutput output;
};

static std::vector<Vec3NormalizeTestData> vec3s_test_data;
static std::vector<Vec3NormalizeTestData> vec3l_test_data;
static std::mutex lock_vec3s;
static std::mutex lock_vec3l;

static void add_test_sample_normalize(Vec3f vec, Vec3f vec_normalized) {
    return;
    std::lock_guard l1(lock_vec3s);
    std::lock_guard l2(lock_vec3l);
    if (std::abs(vec.x) > 1.0f || std::abs(vec.y) > 1.0f || std::abs(vec.z) > 1.0f) {
        if (vec3l_test_data.size() < 100) {
            vec3l_test_data.push_back({{vec}, {vec_normalized}});
        }
    } else if (std::abs(vec.x) > 0.01f && std::abs(vec.y) > 0.01f && std::abs(vec.z) > 0.01f) {
        if (vec3s_test_data.size() < 100) {
            vec3s_test_data.push_back({{vec}, {vec_normalized}});
        }
    }
    if (vec3s_test_data.size() == 100 && vec3l_test_data.size() == 100) {
        rfl::json::save("vec3s_normalize_test_data.json", vec3s_test_data);
        rfl::json::save("vec3l_normalize_test_data.json", vec3l_test_data);
        exit(0);
    }
}

template <typename T>
Vec3<T>& Vec3<T>::normalize(std::source_location loc) {
    auto copy = Vec3<T>(*this);
    T one_over_length = static_cast<T>(1) / this->length();
    this->x *= one_over_length;
    this->y *= one_over_length;
    this->z *= one_over_length;
    if constexpr (std::is_same_v<T, f32>) {
        add_test_sample_normalize(copy, *this);
    }

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


template <typename T>
inline Vec3<T> Vec3<T>::max(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3(
        std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)
    );
}

template <typename T>
inline Vec3<T> Vec3<T>::min(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3(
        std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)
    );
}

template <typename T>
inline Vec3<T> Vec3<T>::abs(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3(std::abs(a.x, b.x), std::abs(a.y, b.y), std::abs(a.z, b.z));
}



