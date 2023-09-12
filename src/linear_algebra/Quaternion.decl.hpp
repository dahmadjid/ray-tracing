#pragma once

#include "linear_algebra/Vec4.hpp"

template <typename T>
struct Vec3;

template<typename T>
struct Quaternion: public Vec4<T> {
    
    Quaternion(const Vec4<T>& v);
    Quaternion(T w, T x, T y, T z);

    Quaternion cross(const Quaternion<T>& rhs) const;
    static Quaternion angle_axis(float angle_radians, const Vec3<T>& unit_axis);

    Quaternion operator-() const;
};