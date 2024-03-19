#include "Material.hpp"
#include <cmath>
#include <optional>
#include "utils/MathUtils.hpp"
#include "utils/Panic.hpp"
#include "math.h"
#include "stdlib.h"
#include "BRDF.hpp"

// #define CHECK_CLAMP(number, a, b) if (number <= a || number >= b) {return std::nullopt;}

//#define CHECK_NAN(number)\
//if (std::isnan(number)) {\
//    panic("number is nan");\    
//}\


#define CHECK_NAN_VEC3(vec)\
if (std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z)) {\
    panic("vec is nan {}", vec);\
}\


namespace RayTracer {

// https://google.github.io/filament/Filament.html
f32 Material::D_GGX(f32 NoH) const {
    f32 f = (NoH * this->alpha2 - NoH) * NoH + 1.0f;
    return this->alpha2 / ((f32)std::numbers::pi * f * f);
}

Vec3f Material::F_Schlick(float u, Vec3f f0) const {
    f32 f = std::pow(1.0f - u, 5.0f);
    return f + f0 * (1.0f - f);
}

f32 Material::V_SmithGGXCorrelated(f32 NoV, f32 NoL) const {
    f32 GGXL = NoV * std::sqrt((-NoL * this->alpha2 + NoL) * NoL + this->alpha2);
    f32 GGXV = NoL * std::sqrt((-NoV * this->alpha2 + NoV) * NoV + this->alpha2);
    return 0.5f / (GGXV + GGXL);
}

Vec3f Material::brdf(f32 NdotV, f32 NdotH, f32 LdotH, f32 NdotL) const {

    // f32 D = D_GGX(NdotH);
    // Vec3f  F = F_Schlick(LdotH, mix(Vec3f(0.04f), this->albedo, this->metallic));
    // f32 V_SmithGGX = V_SmithGGXCorrelated(NdotV, NdotL);

    // Vec3f specular = D * V_SmithGGX * F;
    Vec3f diffuse = this->albedo * INV_PI;
    Vec3f brdf = diffuse;
    return brdf;
}
// cosine weighted sampling
std::tuple<Vec3f, Vec3f, f32> Material::sample(u32& seed, const Vec3f& view_vector, const Vec3f& normal_vector) const {
    f32 phi = 2 * PI * rand_float(seed);
    f32 cos_phi = std::cos(phi);
    f32 sin_phi = std::sin(phi);

    f32 uniform_random = rand_float(seed);
    f32 theta = std::acos(std::sqrt(uniform_random));
    f32 cos_theta = std::cos(theta);
    f32 sin_theta = std::sin(theta);

    Vec3f light_vector = Vec3f(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi).normalize(); 
    if (light_vector.dot(normal_vector) < 0) {
        light_vector = -light_vector;
    }
    Vec3f half_vector = (light_vector + view_vector).normalize();
    f32 pdf = cos_theta / PI;
    return { half_vector, light_vector, pdf };
}

f32 Material::pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& normal) {
    return clamp(normal.dot(sampled_light_dir), 0, 1) / PI;
}


}

