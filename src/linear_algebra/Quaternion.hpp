#pragma once

#include "linear_algebra/Vec4.hpp"

template<typename T>
struct Quaternion: public Vec4<T> {
    
    Quaternion(const Vec4<T>& v): Vec4<T>(v) {}
    Quaternion(T w, T x, T y, T z): Vec4<T>(w, x, y, z) {}

    Quaternion cross(const Quaternion<T>& rhs) const {
        return Quaternion(
            this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z,
            this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y,
            this->w * rhs.y + this->y * rhs.w + this->z * rhs.x - this->x * rhs.z,
            this->w * rhs.z + this->z * rhs.w + this->x * rhs.y - this->y * rhs.x
        );
    }

    static Quaternion angle_axis(float angle_radians, T unit_axis_x, T unit_axis_y, T unit_axis_z) {
        auto half_cos_angle = std::cos(angle_radians / 2.0);
        auto half_sin_angle = std::sin(angle_radians / 2.0);
        return Quaternion(
            half_cos_angle,
            unit_axis_x * half_sin_angle,
            unit_axis_y * half_sin_angle,
            unit_axis_z * half_sin_angle
        );
    }

    Quaternion operator-() const {
        return Quaternion(this->w, -this->x, -this->y, -this->z);
    }
};