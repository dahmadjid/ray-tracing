#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>

template <typename T>
struct Quaternion;

template<typename T>
struct Vec3 {
    T x;
    T y;
    T z;
    Vec3();
    explicit Vec3(T value);

    Vec3(T x, T y, T z);

    T dot(const Vec3& rhs) const;

    /**
     * @brief cross product, returns a new vector
     * 
     * @param rhs 
     * @return Vec3 
     */
    Vec3 cross(const Vec3& rhs) const;

    auto length_squared() const;

    auto length() const;

    /**
     * @brief scales Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    template<typename U>
    Vec3& scale(U factor);

    /**
     * @brief normalizes in place, returns reference to the same Vec3
     * 
     * @return Vec3& 
     */

    Vec3& normalize();
    /**
     * @brief returns a new Vec3 
     * 
     * @return Vec3<uint32_t> 
     */
    Vec3<uint32_t> floor() const;

    /**
     * @brief returns a new Vec3 
     * 
     * @return Vec3<uint32_t> 
     */
    Vec3<uint32_t> ceil() const;

    /**
     * @brief shifts Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    template<typename U>
    Vec3& shift(U factor);
    

    Vec3 operator-() const;

    /**
     * @brief rotate Vec3 in place and returns reference to the same Vec3 
     * 
     * @param factor 
     * @return Vec3& 
     */
    Vec3<T>& rotate(const Quaternion<T>& rotation_quaternion);


    static Vec3<float> random(uint32_t& seed);


    /**
    * @brief Element wise addition
    * @param rhs 
    */
    void operator+=(const Vec3<T>& rhs);


    /**
    * @brief Element wise subtraction
    * @param rhs 
    */
    void operator-=(const Vec3<T>& rhs);


    /**
    * @brief Element wise multiplication
    * @param rhs 
    */
    void operator*=(const Vec3<T>& rhs);


    /**
    * @brief Element wise division
    * @param rhs 
    */
    void operator/=(const Vec3<T>& rhs);

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
