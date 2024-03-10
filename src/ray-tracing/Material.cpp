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

#define PI (f32)std::numbers::pi
#define INV_PI (f32)std::numbers::inv_pi

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

    f32 D = D_GGX(NdotH);
    Vec3f  F = F_Schlick(LdotH, mix(Vec3f(0.04f), this->albedo, this->metallic));
    f32 V_SmithGGX = V_SmithGGXCorrelated(NdotV, NdotL);

    Vec3f specular = D * V_SmithGGX * F;
    Vec3f diffuse = this->albedo * INV_PI;
    Vec3f brdf = diffuse * (- F + 1.0f) * (1.0f - this->metallic) + specular;
    return  brdf * NdotL;
}





std::pair<Vec3f, Vec3f> Material::sample(u32& seed, const Vec3f& ray_direction) const {
    f32 phi = 2 * PI * rand_float(seed);
    f32 cos_phi = std::cos(phi);
    f32 sin_phi = std::sin(phi);


    f32 uniform_random = rand_float(seed);
    f32 theta = std::acos(std::sqrt((1 - uniform_random) / (uniform_random * (this->alpha2 - 1) + 1))); 
    // f32 theta_with_atan = std::atan(this->alpha * std::sqrt(uniform_random) / std::sqrt(1 - uniform_random));
    f32 cos_theta = std::cos(theta);
    f32 sin_theta = std::sin(theta);

    Vec3f half_vector = Vec3f(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi).normalize(); 
    Vec3f light_vector = ray_direction.reflect(half_vector);



	// Vec3f Vh = Vec3f(this->alpha * ray_direction.x, this->alpha * ray_direction.y, ray_direction.z).normalize();
	// // Source: "Sampling Visible GGX Normals with Spherical Caps" by Dupuy & Benyoub

	// // Sample a spherical cap in (-Vh.z, 1]
	// // float phi = 2.0f * PI * u.x;
	// float z = ((1.0f - uniform_random) * (1.0f + Vh.z)) - Vh.z;
	// float sinTheta = sqrt(clamp(1.0f - z * z, 0.0f, 1.0f));
	// float x = sinTheta * cos_phi;
	// float y = sinTheta * sin_phi;

	// // compute halfway direction;
	// Vec3f Nh = Vec3f(x, y, z) + Vh;
    // Vec3f half_vector = Vec3f(this->alpha * Nh.x, this->alpha * Nh.y, std::max(0.0f, Nh.z)).normalize();
    // Vec3f light_vector = -ray_direction.reflect(half_vector);



    // float tmp = wi.x * wi.x + wi.y * wi.y;
    // vec3 w1 = tmp > 0.0f ? vec3(-wi.y, wi.x, 0) * inversesqrt(tmp)
    // : vec3(1, 0, 0);
    // vec3 w2 = cross(wi, w1);
    // // parameterization of the cross section
    // float phi = 2.0f * M_PI * u.x;
    // float r = sqrt(u.y);
    // float t1 = r * cos(phi);
    // float t2 = r * sin(phi);
    // float s = (1.0f + wi.z) / 2.0f;
    // t2 = (1.0f - s) * sqrt(1.0f - t1 * t1) + s * t2;
    // float ti = sqrt(max(1.0f - t1 * t1 - t2 * t2, 0.0f));
    // // reprojection onto hemisphere
    // vec3 wm = t1 * w1 + t2 * w2 + ti * wi;


    return { half_vector, light_vector };
}


// float sampleGGXVNDFReflectionPdf(float alpha, float alphaSquared, float NdotH, float NdotV, float LdotH) {
// 	NdotH = std::max(0.00001f, NdotH);
// 	NdotV = std::max(0.00001f, NdotV);
// 	return (D_GGX(std::max(0.00001f, alphaSquared), NdotH) * Smith_G1_GGX(alpha, NdotV, alphaSquared, NdotV * NdotV)) / (4.0f * NdotV);
// }


f32 Material::pdf(f32 NdotH, f32 NdotV) const {
    NdotH = std::max(0.00001f, NdotH);
	NdotV = std::max(0.00001f, NdotV);
	return (D_GGX(NdotH) * Smith_G1_GGX(NdotV)) / (4.0f * NdotV);
    // f32 d = D_GGX(NdotH); 
    // return d * NdotH / 4 / VdotH;
}

f32 Material::Smith_G1_GGX(f32 NdotK) const {
    f32 NdotK2 = NdotK * NdotK;
	return 2.0f / (sqrt(((this->alpha2 * (1.0f - NdotK2)) + NdotK2) / NdotK2) + 1.0f);
}

}

