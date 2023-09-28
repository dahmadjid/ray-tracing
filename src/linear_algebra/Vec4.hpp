#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <fmt/format.h>
#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "utils/MathUtils.hpp"

template<typename T>
struct Vec4 {
    T w;
    T x;
    T y;
    T z;
    
    Vec4(): w(0), x(0), y(0), z(0) {}
    Vec4(T w, T x, T y, T z): w(w), x(x), y(y), z(z) {}
    Vec4(T w, const Vec3<T>& v): w(w), x(v.x), y(v.y), z(v.z) {}
    Vec4(const Vec3<T>& v, T z): w(v.x), x(v.y), y(v.z), z(z) {}

    T dot(const Vec4& rhs) {
        return this->w * rhs.w + this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
    }

    auto length_squared() {
        return this->dot(*this);
    }

    auto length() {
        return std::sqrt(this->length_squared());
    }

    /**
     * @brief scales Vec4 in place and returns reference to the same Vec4 
     * 
     * @param factor 
     * @return Vec4& 
     */
    template<typename U>
    Vec4& scale(U factor) {
        this->w *= factor;
        this->x *= factor;
        this->y *= factor;
        this->z *= factor;
        return *this;
    }

    /**
     * @brief normalizes in place, returns reference to the same Vec4
     * 
     * @return Vec4& 
     */
    Vec4& normalize() {
        T one_over_length = static_cast<T>(1) / this->length();
        this->w *= one_over_length;
        this->x *= one_over_length;  
        this->y *= one_over_length;  
        this->z *= one_over_length;
        return *this;  
    }
    
    /**
     * @brief returns a new Vec4 
     * 
     * @return Vec4<uint32_t> 
     */
    template<typename CastType>
    Vec4<CastType> floor() {
        return Vec4<CastType>(
            static_cast<CastType>(std::floor(this->w)),
            static_cast<CastType>(std::floor(this->x)),
            static_cast<CastType>(std::floor(this->y)),
            static_cast<CastType>(std::floor(this->z))
        );
    }

    /**
     * @brief returns a new Vec4 
     * 
     * @return Vec4<uint32_t> 
     */
    template<typename CastType>
    Vec4<CastType> ceil() {
        return Vec4<CastType>(
            static_cast<CastType>(std::ceil(this->w)),
            static_cast<CastType>(std::ceil(this->x)),
            static_cast<CastType>(std::ceil(this->y)),
            static_cast<CastType>(std::ceil(this->z))
        );
    }

    /**
     * @brief shifts Vec4 in place and returns reference to the same Vec4 
     * 
     * @param factor 
     * @return Vec4& 
     */
    template<typename U>
    Vec4& shift(U factor) {
        this->w += factor;
        this->x += factor;
        this->y += factor;
        this->z += factor;
        return *this;
    }

    Vec4 operator-() const {
        return Vec4(-this->w, -this->x, -this->y, -this->z);
    }
    

    static Vec4<float> random(uint32_t& seed) {
        return Vec4<float>(
            rand_float(seed) * 2 - 1,
            rand_float(seed) * 2 - 1,
            rand_float(seed) * 2 - 1,
            rand_float(seed) * 2 - 1
        );
    }

    template<typename U>
    Vec4<U> cast() {
        return Vec4<U>(
            static_cast<U>(this->w),
            static_cast<U>(this->x),
            static_cast<U>(this->y),
            static_cast<U>(this->z)
        );
    }

    /**
    * @brief Element wise addition
    * @param rhs 
    */
    void operator+=(const Vec4<T>& rhs) {
        this->w += rhs.w; 
        this->x += rhs.x; 
        this->y += rhs.y; 
        this->z += rhs.z;
    }


    /**
    * @brief Element wise subtraction
    * @param rhs 
    */
    void operator-=(const Vec4<T>& rhs) {
        this->w -= rhs.w; 
        this->x -= rhs.x; 
        this->y -= rhs.y; 
        this->z -= rhs.z;
    }


    /**
    * @brief Element wise multiplication
    * @param rhs 
    */
    void operator*=(const Vec4<T>& rhs) {
        this->w *= rhs.w; 
        this->x *= rhs.x; 
        this->y *= rhs.y; 
        this->z *= rhs.z;
    }


    /**
    * @brief Element wise division
    * @param rhs 
    */
    void operator/=(const Vec4<T>& rhs) {
        this->w /= rhs.w; 
        this->x /= rhs.x; 
        this->y /= rhs.y; 
        this->z /= rhs.z;
    }
};

/**
 * @brief Element wise addition
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec4<T> 
 */
template<typename T>
static Vec4<T> operator+(const Vec4<T>& lhs, const Vec4<T>& rhs) {
    return Vec4(lhs.w + rhs.w, lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}


/**
 * @brief Element wise subtraction
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec4<T> 
 */
template<typename T>
static Vec4<T> operator-(const Vec4<T>& lhs, const Vec4<T>& rhs) {
    return Vec4(lhs.w - rhs.w, lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


/**
 * @brief Element wise multiplication
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec4<T> 
 */
template<typename T>
static Vec4<T> operator*(const Vec4<T>& lhs, const Vec4<T>& rhs) {
    return Vec4(lhs.w * rhs.w, lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}


/**
 * @brief Element wise division
 * 
 * @tparam T 
 * @param lhs 
 * @param rhs 
 * @return Vec4<T> 
 */
template<typename T>
static Vec4<T> operator/(const Vec4<T>& lhs, const Vec4<T>& rhs) {
    return Vec4(lhs.w / rhs.w, lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

template<typename T>
static bool operator==(const Vec4<T>& lhs, const Vec4<T>& rhs) {
    return lhs.w == rhs.w && lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}



template <typename T>
struct fmt::formatter<Vec4<T>> : formatter<std::string> {
  auto format(const Vec4<T>& vec, fmt::format_context& ctx) const {
    std::string out = fmt::format("Vec4({}, {}, {}, {})", vec.w, vec.x, vec.y, vec.z);
    return formatter<std::string>::format(out, ctx);
  }
};