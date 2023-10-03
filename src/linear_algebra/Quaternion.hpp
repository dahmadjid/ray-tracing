#pragma once

#include "linear_algebra/Quaternion.decl.hpp"

template<typename T>
Quaternion<T>::Quaternion(const Vec4<T>& v): Vec4<T>(v) {}

template<typename T>
Quaternion<T>::Quaternion(T w, T x, T y, T z): Vec4<T>(w, x, y, z) {}

 
template<typename T>
Quaternion<T> Quaternion<T>::cross(const Quaternion<T>& rhs) const {
    return Quaternion(
        this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z,
        this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y,
        this->w * rhs.y + this->y * rhs.w + this->z * rhs.x - this->x * rhs.z,
        this->w * rhs.z + this->z * rhs.w + this->x * rhs.y - this->y * rhs.x
    );
}

template<typename T>
Quaternion<T> Quaternion<T>::angle_axis(T angle_radians, const Vec3<T>& unit_axis) {
    T half_cos_angle = (T)std::cos(angle_radians / 2.0);
    T half_sin_angle = (T)std::sin(angle_radians / 2.0);
    return Quaternion(
        half_cos_angle,
        unit_axis.x * half_sin_angle,
        unit_axis.y * half_sin_angle,
        unit_axis.z * half_sin_angle
    );
}

template<typename T>
Quaternion<T> Quaternion<T>::operator-() const {
    return Quaternion(this->w, -this->x, -this->y, -this->z);
}