#pragma once

#include <cmath>
#include <cstdint>
#include "linear_algebra/Quaternion.hpp"
#include <format>
#include <iostream>

template<typename T>
struct Vec3 {
    T x;
    T y;
    T z;

    Vec3(T x, T y, T z): x(x), y(y), z(z) {}

    T dot(const Vec3& rhs) const {
        return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
    }

    /**
     * @brief cross product, returns a new vector
     * 
     * @param rhs 
     * @return Vec3 
     */
    Vec3 cross(const Vec3& rhs) const {
        return Vec3(
            this->y * rhs.z - this->z * rhs.y,
            this->z * rhs.x - this->x * rhs.z,
            this->x * rhs.y - this->y * rhs.x
        );
    }

    auto length_squared() const {
        return this->dot(*this);
    }

    auto length() const {
        return std::sqrt(this->length_squared());
    }

    /**
     * @brief scales Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    Vec3& scale(T factor) {
        this->x *= factor;
        this->y *= factor;
        this->z *= factor;
        return *this;
    }

    /**
     * @brief normalizes in place, returns reference to the same Vec3
     * 
     * @return Vec3& 
     */
    Vec3& normalize() {
        T one_over_length = static_cast<T>(1) / this->length();
        this->x *= one_over_length;  
        this->y *= one_over_length;  
        this->z *= one_over_length;
        return *this;  
    }
    
    /**
     * @brief returns a new Vec3 
     * 
     * @return Vec3<uint32_t> 
     */
    Vec3<uint32_t> floor() const {
        return Vec3<uint32_t>(
            static_cast<uint32_t>(std::floor(this->x)),
            static_cast<uint32_t>(std::floor(this->y)),
            static_cast<uint32_t>(std::floor(this->z))
        );
    }

    /**
     * @brief returns a new Vec3 
     * 
     * @return Vec3<uint32_t> 
     */
    Vec3<uint32_t> ceil() const {
        return Vec3<uint32_t>(
            static_cast<uint32_t>(std::ceil(this->x)),
            static_cast<uint32_t>(std::ceil(this->y)),
            static_cast<uint32_t>(std::ceil(this->z))
        );
    }

    /**
     * @brief shifts Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    Vec3& shift(T factor) {
        this->x += factor;
        this->y += factor;
        this->z += factor;
        return *this;
    }
    

    Vec3 operator-() const {
        return Vec3(-this->x, -this->y, -this->z);
    }

    /**
     * @brief rotate Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    Vec3<T>& rotate(const Quaternion<T>& rotation_quaternion) {
        auto quat_to_rotate = Quaternion<T>(static_cast<T>(0), this->x, this->y, this->z);
        auto result = rotation_quaternion.cross(quat_to_rotate).cross(-rotation_quaternion);
        this->x = result.x;
        this->y = result.y;
        this->z = result.z;
        return *this;
    }

};

/**
 * @brief Element wise addition
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec3<T> 
 */
template<typename T>
static Vec3<T> operator+(const Vec3<T>& lhs, const Vec3<T>& rhs) {
    return Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}


/**
 * @brief Element wise subtraction
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec3<T> 
 */
template<typename T>
static Vec3<T> operator-(const Vec3<T>& lhs, const Vec3<T>& rhs) {
    return Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


/**
 * @brief Element wise multiplication
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec3<T> 
 */
template<typename T>
static Vec3<T> operator*(const Vec3<T>& lhs, const Vec3<T>& rhs) {
    return Vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}


/**
 * @brief Element wise division
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec3<T> 
 */
template<typename T>
static Vec3<T> operator/(const Vec3<T>& lhs, const Vec3<T>& rhs) {
    return Vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

template<typename T>
static bool operator==(const Vec3<T>& lhs, const Vec3<T>& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}
