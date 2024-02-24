#include "Material.hpp"
#include <optional>
#include "utils/Panic.hpp"
#include "math.h"
#include "stdlib.h"
#include "BRDF.hpp"

#define CHECK_CLAMP(number, a, b)\
if (number <= a || number >= b) {\
    return std::nullopt;\
    /*panic("number {} is not in range", #number)*/;\
}\

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


namespace { // annonymous namespace for static linking
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
}

namespace PBR {
namespace { // annonymous namespace for static linking

f32 clamp(f32 n, f32 min, f32 max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

f32 mix(f32 x, f32 y, f32 a) {
    return x * (1 - a) + y * a;
}

Vec3f mix(const Vec3f& x, const Vec3f& y, f32 a) {
    return x * (1 - a) + y * a;
}


Vec3f schlick_fresnel(f32 HdotV, const Vec3f& spec_color) {
    
    f32 m = 1 - HdotV;
    f32 m2 = m * m;
    return spec_color + (- spec_color + 1.0f) * m2 * m2 * m;
}

f32 ggx(f32 NdotH, f32 a) {
    f32 a2 = a * a;
    f32 deno = std::max(1 + (a2 - 1) * NdotH * NdotH, 0.0001f);
    return a2 * INV_PI / (deno * deno);
}

f32 geometry_smith_ggx(f32 NdotV, f32 NdotL, f32 a) {
    f32 k = 0.5 * a;
    f32 view_term = NdotV / std::max(NdotV * (1 - k) + k, 0.0001f);
    f32 light_term = NdotL / std::max(NdotL * (1 - k) + k, 0.0001f);
    return view_term * light_term;
}

Vec3f mon2lin(const Vec3f& vec) {
    return Vec3f(std::pow(vec.x, 2.2f), std::pow(vec.y, 2.2f), std::pow(vec.z, 2.2f));
}
//https://alextardif.com/PhysicallyBasedRendering.html
Vec3f BRDF(const Vec3f& L, const Vec3f& V, const Vec3f& N, const Vec3f& H, const RayTracer::Material& material) {
    f32 NdotL = clamp(N.dot(L), 0, 1);
    f32 NdotV = clamp(N.dot(V), 0, 1);
    f32 NdotH = clamp(N.dot(H), 0, 1);
    f32 VdotH = clamp(V.dot(H), 0, 1);

    f32 alpha = material.roughness * material.roughness;
    Vec3f F = schlick_fresnel(VdotH, mix(Vec3f(0.05), material.albedo, material.metallic));
    f32 G = geometry_smith_ggx(NdotV, NdotL, alpha);
    f32 D = ggx(NdotH, alpha);

    Vec3f cook_torrance_nume = D * G * F;
    f32 cook_torrance_deno = NdotV * NdotL * 4.0f;


    Vec3f cook_torrance = cook_torrance_nume / std::max(cook_torrance_deno, 0.0001f);
    
    Vec3f Kd = - F + 1.0f;
    Vec3f diffuse = material.albedo * INV_PI * Kd ;
    Vec3f brdf = diffuse + cook_torrance;
    return brdf;
}
}
}



namespace RayTracer {
        
Vec3f Material::brdf(const Vec3f& L, const Vec3f& V, const Vec3f& N, const Vec3f& H) const {
    return PBR::BRDF(L, V, N, H, *this);
}

std::tuple<Vec3f, f32> Material::sample(u32& seed, const Vec3f& V) const {
    f32 uniform_random = rand_float(seed);
    f32 phi = 2 * PI * rand_float(seed);
    f32 nume = (1 - uniform_random);
    f32 deno = uniform_random * (this->roughness * this->roughness - 1) + 1;
    f32 cos_theta = std::sqrt(nume / deno);
    f32 sin_theta = std::sqrt(1 - cos_theta * cos_theta);
    //f32 theta = std::acosf(cos_theta);
    f32 cos_phi = std::cosf(phi);
    f32 sin_phi = std::sqrt(1 - cos_theta * cos_theta);

    Vec3f H(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
    f32 pdf = this->pdf(cos_theta, sin_theta);
    f32 HdotV = H.dot(V);
    f32 inv_pdf = (4 * HdotV) / pdf;
    return { H.normalize(), inv_pdf};
}

f32 Material::pdf(f32 cos_theta, f32 sin_theta) const {
    f32 deno = alpha_two_minus_one * cos_theta * cos_theta + 1;
    return (this->two_alpha_two * cos_theta * sin_theta) / (deno * deno);
}

}

