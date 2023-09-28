#pragma once


#include "linear_algebra/Vec3.hpp"

namespace RayTracer {

struct Ray {
    Vec3<f32> origin;
    Vec3<f32> direction;
};

}
