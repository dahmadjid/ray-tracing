#pragma once

#include <cmath>
#include <cstdint>

template<typename T>
struct Vec4 {
    T w;
    T x;
    T y;
    T z;
    
    Vec4(): w(0), x(0), y(0), z(0) {}
    Vec4(T w, T x, T y, T z): w(w), x(x), y(y), z(z) {}

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
    Vec4& scale(T factor) {
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
    Vec4<uint32_t> floor() {
        return Vec4<uint32_t>(
            static_cast<uint32_t>(std::floor(this->w)),
            static_cast<uint32_t>(std::floor(this->x)),
            static_cast<uint32_t>(std::floor(this->y)),
            static_cast<uint32_t>(std::floor(this->z))
        );
    }

    /**
     * @brief returns a new Vec4 
     * 
     * @return Vec4<uint32_t> 
     */
    Vec4<uint32_t> ceil() {
        return Vec4<uint32_t>(
            static_cast<uint32_t>(std::ceil(this->w)),
            static_cast<uint32_t>(std::ceil(this->x)),
            static_cast<uint32_t>(std::ceil(this->y)),
            static_cast<uint32_t>(std::ceil(this->z))
        );
    }

    /**
     * @brief shifts Vec4 in place and returns reference to the same Vec4 
     * 
     * @param factor 
     * @return Vec4& 
     */
    Vec4& shift(T factor) {
        this->w += factor;
        this->x += factor;
        this->y += factor;
        this->z += factor;
        return *this;
    }

    Vec4 operator-() const {
        return Vec4(-this->w, -this->x, -this->y, -this->z);
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
