#pragma once

#include "linear_algebra/Vec3.hpp"

struct ONBTestData {
    Vec3f input_normal;
    Vec3f input_vec;
    Vec3f output_vec;
    Vec3f debug_u;
    Vec3f debug_v;
};

static u32 seed = 356789;
static std::mutex onb_test_data_mutex;
static ONBTestData onb_test_cases[1000];
static int i = 0;

// rtweekend thank you
class ONB {
public:
    ONB(const Vec3f& normal) {
        Vec3f a = (fabs(normal.x) > 0.9) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
        Vec3f v = normal.cross(a);
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
        auto out = (a.x * u() + a.y * v() + a.z * w()).normalize();
        if (rand_float(seed) > 0.995f) {
            onb_test_data_mutex.lock();
            onb_test_cases[i] = {
                .input_normal = w(),
                .input_vec = a,
                .output_vec = out,
                .debug_u = u(),
                .debug_v = v(),
            };
            i++;
            onb_test_data_mutex.unlock();
        }
        if (i == 1000) {
            rfl::json::save("onb_test_data.json", onb_test_cases);
            exit(1);
        }
        return out;
    }

    Vec3f axis[3];
};