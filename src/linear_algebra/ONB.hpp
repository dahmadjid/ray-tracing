#pragma once

#include "linear_algebra/Vec3.hpp"

// rtweekend thank you
class ONB {
public:
    ONB(const Vec3f& normal) {
        Vec3f a = (fabs(normal.x) > 0.9) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
        Vec3f v = normal.cross(a);
        v.normalize();
        Vec3f u = normal.cross(v);
        axis[0] = u;
        axis[1] = v;
        axis[2] = normal;
    }

    inline Vec3f operator[](int i) const {
        return axis[i];
    }

    inline Vec3f& operator[](int i) {
        return axis[i];
    }

    inline Vec3f u() const {
        return axis[0];
    }

    inline Vec3f v() const {
        return axis[1];
    }

    inline Vec3f w() const {
        return axis[2];
    }

    inline Vec3f local(const Vec3f& a) const {
        return a.x * u() + a.y * v() + a.z * w();
    }

    Vec3f axis[3];
};