#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <fmt/core.h>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_relational.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/matrix.hpp>
#include <numbers>
#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "linear_algebra/Mat3.hpp"
#include "linear_algebra/Mat4.hpp"
#include "linear_algebra/Quaternion.hpp"
#include <format>
#include <iostream>
#include "utils/MathUtils.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <type_traits>
#include <glm/gtx/string_cast.hpp>


using namespace glm;

template<typename T, typename U>
void equal(T a, U b) {
    REQUIRE_THAT(a, Catch::Matchers::WithinAbs(b, 0.001));
}

template<typename T, typename U>
void vec3_equal(T v1, U v2) {
    equal(v1.x, v2.x);
    equal(v1.y, v2.y);
    equal(v1.z, v2.z);
}

template<typename T, typename U>
void vec4_equal(T v1, U v2) {
    if constexpr (std::is_same<T, U>()) {
        equal(v1.x, v2.x);
        equal(v1.y, v2.y);
        equal(v1.z, v2.z);
        equal(v1.w, v2.w);
    } else {
        equal(v1.w, v2.x);
        equal(v1.x, v2.y);
        equal(v1.y, v2.z);
        equal(v1.z, v2.w);
    }
}

void mat3_equal(Mat3<f32> m1, mat3 m2) {
    for (u32 i = 0; i < 3; i++) {
        for (u32 j = 0; j < 3; j++) {
            equal(m1.get(i, j), m2[i][j]);
        }
    }
}

void mat4_equal(Mat4<f32> m1, mat4 m2) {
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            equal(m1.get(i, j), m2[i][j]);
        }
    }
}


// Vec4 tests


TEST_CASE("VEC4: DOT") {

    auto a = Vec4<f32>(19.2, 12., 112., 42.);
    auto b = Vec4<f32>(20.5, 33., 32., 3.1231);
    auto c = a.dot(b);

    auto a_glm = vec4(19.2, 12., 112., 42.);
    auto b_glm = vec4(20.5, 33., 32., 3.1231);
    auto c_glm = glm::dot(a_glm, b_glm);

    equal(c, c_glm);
}



TEST_CASE("VEC4: LENGTH") {
    auto a = Vec4<f32>(19.2, 12., 112., 42.);
    auto a_glm = vec4(19.2, 12., 112., 42.);
    equal(a.length(), glm::length(a_glm));
}


TEST_CASE("VEC4: SHIFT SCALE") {
    auto a = Vec4<f32>(19.2, 12., 112., 42.);
    auto a_glm = vec4(19.2, 12., 112., 42.);
    vec4_equal(Vec4(a).scale(1.921319f), a_glm * 1.921319f);
    vec4_equal(Vec4(a).shift(1.921319f), a_glm + 1.921319f);
    vec4_equal(Vec4(a).scale(1.f/1.921319f), a_glm / 1.921319f);
    vec4_equal(Vec4(a).shift(-1.921319f), a_glm - 1.921319f);
}


TEST_CASE("VEC4: ADD MINUS MULTIPLY DIVIDE") {
    auto a = Vec4<f32>(19.2, 12., 112., 42.);
    auto b = Vec4<f32>(20.5, 33., 32., 3.1231);

    auto a_glm = vec4(19.2, 12., 112., 42.);
    auto b_glm = vec4(20.5, 33., 32., 3.1231);

    vec4_equal(a + b,  a_glm + b_glm);
    vec4_equal(a - b,  a_glm - b_glm);
    vec4_equal(a * b,  a_glm * b_glm);
    vec4_equal(a / b,  a_glm / b_glm);
}





// Vec3 tests

TEST_CASE("VEC3: DOT") {
    auto a = Vec3(1.5, 2.5, 10.);
    auto b = Vec3(20.5, 3., 3.);
    auto c = a.dot(b);

    auto a_glm = vec3(1.5, 2.5, 10.);
    auto b_glm = vec3(20.5, 3., 3.);
    auto c_glm = glm::dot(a_glm, b_glm);

    equal(c, c_glm);
}


TEST_CASE("VEC3: CROSS") {
    auto a = Vec3(1.5, 2.5, 10.);
    auto b = Vec3(20.5, 3., 3.);
    auto c = a.cross(b);

    auto a_glm = vec3(1.5, 2.5, 10.);
    auto b_glm = vec3(20.5, 3., 3.);
    auto c_glm = glm::cross(a_glm, b_glm);

    vec3_equal(c, c_glm);
}



TEST_CASE("VEC3: LENGTH") {
    auto a = Vec3(5.0, 3.0, 1.0);
    auto a_glm = vec3(5.0, 3.0, 1.0);
    equal(a.length(), glm::length(a_glm));
}


TEST_CASE("VEC3: SHIFT SCALE") {
    auto a = Vec3<f32>(5.0, 3.0, 1.0);
    auto a_glm = vec3(5.0, 3.0, 1.0);
    vec3_equal(Vec3(a).scale(1.921319), a_glm * 1.921319f);
    vec3_equal(Vec3(a).shift(1.921319), a_glm + 1.921319f);
    vec3_equal(Vec3(a).scale(1/1.921319), a_glm / 1.921319f);
    vec3_equal(Vec3(a).shift(-1.921319), a_glm - 1.921319f);
}


TEST_CASE("VEC3: ADD MINUS MULTIPLY DIVIDE") {
    auto a = Vec3(1.5, 2.5, 10.);
    auto b = Vec3(20.5, 3., 3.);

    auto a_glm = vec3(1.5, 2.5, 10.);
    auto b_glm = vec3(20.5, 3., 3.);

    vec3_equal(a + b,  a_glm + b_glm);
    vec3_equal(a - b,  a_glm - b_glm);
    vec3_equal(a * b,  a_glm * b_glm);
    vec3_equal(a / b,  a_glm / b_glm);
}





TEST_CASE("VEC3: QUATERNION ROTATION MATRIX") {
    auto a = Vec3<f32>(1., 0., 0.);
    // rotation matrix of 45 degrees from the internet by the (1, 1, 0) axis.
    auto rotation_matrix = Mat3<f32>({
        0.8535534,  0.1464466,  0.5000000,
        0.1464466,  0.8535534, -0.500000,
        -0.5000000,  0.5000000,  0.7071068 
    });
    
    auto axis = Vec3<f32>(1., 1., 0.).normalize();
    auto rotated_by_quat = Vec3(a).rotate(
        Quaternion<f32>::angle_axis(-to_radians(45.0f), axis)
    );
    auto rotated_by_matrix = rotation_matrix.vec_mul(a);
    vec3_equal(rotated_by_quat, rotated_by_matrix);
}




TEST_CASE("ROTATION") {
    f32 pitchDelta = 0.2001231;
    f32 yawDelta = 0.234;

    auto v1 = Vec3<f32>(1.5, 2.5, 10.);
    auto v2 = Vec3<f32>(3., 2., 7.);

    auto q1 = Quaternion<f32>::angle_axis(-pitchDelta, v1);
    auto q2 = Quaternion<f32>::angle_axis(-yawDelta, Vec3(0.f, 1.0f, 0.0f));

    auto q3 = q1.cross(q2).normalize();

    auto v3 = v2.rotate(q3);


    auto v1_glm = vec3(1.5, 2.5, 10.);
    auto v2_glm = vec3(3., 2., 7.);

    auto q1_glm = glm::angleAxis(-pitchDelta, v1_glm);
    auto q2_glm = glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f));

    glm::quat q3_glm = glm::normalize(
        glm::cross(
           q1_glm,
           q2_glm 
        )
    );

    auto v3_glm = glm::rotate(q3_glm, v2_glm);

    vec3_equal(v3, v3_glm);
    vec3_equal(q1, q1_glm);
    vec3_equal(q2, q2_glm);
    vec3_equal(q3, q3_glm);
}



// Mat3 tests

TEST_CASE("MAT3: MAT-MAT") {
    Mat3<f32> a{.data={1, 2, 3, 4, 5, 6, 7, 8, 9}};
    Mat3<f32> b{.data={1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123}};
    
    mat3 a_glm(1, 2, 3, 4, 5, 6, 7, 8, 9);
    mat3 b_glm(1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123);

    mat3_equal(a.mat_mul(b), a_glm * b_glm);
}


// Mat3 tests

TEST_CASE("MAT3: MAT-VEC") {
    Mat3<f32> a{.data={1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123}};
    auto b = Vec3<f32>(1.5, 2.5, 10.);
    
    mat3 a_glm(1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123);
    auto b_glm = vec3(1.5, 2.5, 10.);

    vec3_equal(a.vec_mul(b), a_glm * b_glm);
}



TEST_CASE("MAT3: inverse") {
    Mat3<f32> a{.data={1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123}};
    
    mat3 a_glm(1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123);
    mat3_equal(a.inverse().value(), glm::inverse(a_glm));
}


TEST_CASE("MAT3: inverse") {
    Mat3<f32> a{.data={1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123}};
    
    mat3 a_glm(1.123, 2.12, 3.45, 4.32, 5.997, 6.12, 72.12, 8.23, 9.123);
    mat3_equal(a.inverse().value(), glm::inverse(a_glm));
}