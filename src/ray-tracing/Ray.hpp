#pragma once


#include "linear_algebra/Vec3.decl.hpp"

namespace RayTracer {

struct Ray {
    Vec3<float> origin;
    Vec3<float> direction;
};

}