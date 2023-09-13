#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <numbers>
#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "linear_algebra/Mat3.hpp"
#include "linear_algebra/Mat4.hpp"
#include "linear_algebra/Quaternion.hpp"
#include <format>
#include <iostream>
#include "utils/MathUtils.h"


TEST_CASE("VEC3: DOT") {
    auto a = Vec3(1.5, 2.5, 10.);
    auto b = Vec3(20.5, 3., 3.);

    auto c = a.dot(b);

    REQUIRE(c == 68.25);
}


TEST_CASE("VEC3: CROSS") {
    auto a = Vec3(1.5, 2.5, 10.);
    auto b = Vec3(20.5, 3., 3.);

    auto c = a.cross(b);

    REQUIRE(c == Vec3(-22.5, 200.5, -46.75));
}


TEST_CASE("VEC3: LENGTH") {
    auto a = Vec3(5.0, 3.0, 1.0);
    auto l = a.length();

    REQUIRE(l == std::sqrt(35.0));
}


TEST_CASE("VEC3: QUATERNION ROTATION") {
    auto a = Vec3<float>(1., 0., 0.);
    // rotation matrix of 45 degrees from the internet by the (1, 1, 0) axis.
    auto rotation_matrix = Mat3<float>({
        0.8535534,  0.1464466,  0.5000000,
        0.1464466,  0.8535534, -0.500000,
        -0.5000000,  0.5000000,  0.7071068 
    });
    
    auto axis = Vec3<float>(1., 1., 0.).normalize();
    auto rotated_by_quat = Vec3(a).rotate(
        Quaternion<float>::angle_axis(to_radians(45.0f), axis)
    );
    auto rotated_by_matrix = rotation_matrix.vec_mul(a);

    REQUIRE_THAT(rotated_by_matrix.x, Catch::Matchers::WithinAbs(rotated_by_quat.x, 0.0000001));
    REQUIRE_THAT(rotated_by_matrix.y, Catch::Matchers::WithinAbs(rotated_by_quat.y, 0.0000001));
    REQUIRE_THAT(rotated_by_matrix.z, Catch::Matchers::WithinAbs(rotated_by_quat.z, 0.0000001));

}
