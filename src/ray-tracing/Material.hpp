#pragma once

#include "utils/MathUtils.hpp"
#include <chrono>
#include <fmt/core.h>
#include <linear_algebra/Vec3.hpp>
#include <numbers>
#include <optional>
#include <thread>
namespace RayTracer {

#define PI (f32)std::numbers::pi
#define INV_PI (f32)std::numbers::inv_pi

inline f32 clamp(f32 n, f32 min, f32 max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

inline f32 mix(f32 x, f32 y, f32 a) {
    return x * (1 - a) + y * a;
}

inline Vec3f mix(const Vec3f& x, const Vec3f& y, f32 a) {
    return x * (1 - a) + y * a;
}

struct MaterialParams {
    Vec3f albedo;
    f32 roughness = 0.5f;
    f32 metallic = 0.0f;
    f32 emission_power = 0.0f;
};

class Material: public MaterialParams {
public:
    Material(const MaterialParams& params): MaterialParams(params) {
        this->alpha = std::max(this->roughness * this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
        this->two_alpha_two = 2 * this->alpha2;
        this->alpha_two_minus_one = this->alpha2 - 1;
    }

    void update_roughness(f32 val) {
        val = clamp(val, 0.001f, 1.0f);
        this->roughness = val;
        this->alpha = std::max(this->roughness * this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
        this->two_alpha_two = 2 * this->alpha2;
        this->alpha_two_minus_one = this->alpha2 - 1;
        fmt::println("new roughness: {}", this->roughness);
    }

    // auto [half_vector, light_vector, pdf]
    std::tuple<Vec3f, Vec3f, f32> sample(u32& seed, const Vec3f& view_vector, const Vec3f& normal_vector) const;
    f32 pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal);
    
    Vec3f brdf(f32 NdotV, f32 NdotH, f32 LdotH, f32 NdotL) const;
    inline Vec3f get_emission() {
        return Vec3(this->albedo).scale(emission_power);
    };

    f32 D_GGX(f32 NoH) const;
    Vec3f F_Schlick(float u, Vec3f f0) const;
    f32 V_SmithGGXCorrelated(f32 NoV, f32 NoL) const;

    // NdotL or NdotV
    f32 Smith_G1_GGX(f32 NdotK) const;

private:
    f32 alpha;
    f32 alpha2;
    f32 two_alpha_two;
    f32 alpha_two_minus_one;
};
}