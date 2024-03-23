#include "Material.hpp"

#include <fmt/core.h>

#include <cmath>

#include "linear_algebra/ONB.hpp"
#include "stdlib.h"
#include "utils/MathUtils.hpp"
#include "utils/Panic.hpp"

#define CHECK_NAN_VEC3(vec)                                            \
    if (std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z)) { \
        panic("vec is nan {}", vec);                                   \
    }

#define CHECK_NEG_VEC3(vec)                    \
    if (vec.x < 0 || vec.y < 0 || vec.z < 0) { \
        panic("vec is neg {}", vec);           \
    }

namespace RayTracer {

std::tuple<Vec3f, Vec3f, f32> Material::sample(u32& seed, const Vec3f& view_vector, const Vec3f& normal_vector) const {
    f32 r1 = rand_float(seed);
    f32 r2 = rand_float(seed);

    f32 phi = 2 * PI * r2;
    f32 cos_phi = std::cos(phi);
    f32 sin_phi = std::sin(phi);
    if (this->metallic == 0) {
        f32 sin_theta = std::sqrt(r1);

        f32 x = cos_phi * sin_theta;
        f32 y = sin_phi * sin_theta;
        f32 z = std::sqrt(1 - r1);
        ONB onb{normal_vector};
        Vec3f light_vector = onb.local(Vec3f(x, y, z));
        f32 pdf = z / PI;
        Vec3f half_vector = (light_vector + view_vector).normalize();
        return {light_vector, half_vector, pdf};
    } else {
        Vec3f Vh = Vec3f(this->alpha * view_vector.x, this->alpha * view_vector.y, view_vector.z).normalize();
        float z = ((1.0f - r1) * (1.0f + Vh.z)) - Vh.z;
        float sinTheta = std::sqrt(clamp(1.0f - z * z, 0.0f, 1.0f));
        float x = sinTheta * cos_phi;
        float y = sinTheta * sin_phi;

        // compute halfway direction;
        Vec3f Nh = Vec3f(x, y, z) + Vh;
        ONB onb{normal_vector};
        Vec3f half_vector = onb.local(Vec3f(this->alpha * Nh.x, this->alpha * Nh.y, std::max(0.0f, Nh.z)).normalize());

        Vec3f light_vector = -view_vector.reflect(half_vector);

        f32 NdotH = half_vector.dot(normal_vector);
        f32 NdotV = view_vector.dot(normal_vector);
        f32 pdf = GTR2(NdotH) * smithG_GGX(NdotV) / (4.0f * NdotV);

        return {light_vector, half_vector, pdf};
    }
}

Vec3f Material::brdf(f32 NdotV, f32 NdotH, f32 LdotH, f32 NdotL) const {
    if (NdotL < 0 || NdotV < 0) {
        panic("NdotL = {}, NdotV = {}", NdotL, NdotV);
        return Vec3f();
    };

    Vec3f Cdlin = mon2lin(this->albedo);
    f32 Cdlum = .3f * Cdlin[0] + .6f * Cdlin[1] + .1f * Cdlin[2];  // luminance approx.

    Vec3f Ctint = Cdlum > 0 ? Cdlin / Cdlum : Vec3f(1);  // normalize lum. to isolate hue+sat
    Vec3f Cspec0 = mix(specular * .08f * mix(Vec3f(1), Ctint, specularTint), Cdlin, metallic);
    Vec3f Csheen = mix(Vec3f(1), Ctint, sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    f32 FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    f32 Fd90 = 0.5f + 2.0f * LdotH * LdotH * roughness;
    f32 Fd = mix(1.0f, Fd90, FL) * mix(1.0f, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    f32 Fss90 = LdotH * LdotH * roughness;
    f32 Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    f32 ss = 1.25f * (Fss * (1.0f / (NdotL + NdotV) - .5f) + .5f);

    // specular
    f32 Ds = GTR2(NdotH);
    f32 FH = SchlickFresnel(LdotH);
    Vec3f Fs = mix(Cspec0, Vec3f(1), FH);
    f32 Gs;
    Gs = smithG_GGX(NdotL);
    Gs *= smithG_GGX(NdotV);

    // sheen
    Vec3f Fsheen = FH * sheen * Csheen;

    return ((1 / PI) * mix(Fd, ss, subsurface) * Cdlin + Fsheen) * (1 - metallic) + Gs * Fs * Ds;
}

f32 Material::pdf(f32 NdotH, f32 NdotL, f32 NdotV) {
    if (this->metallic == 0) {
        return NdotL / PI;
    } else {
        return GTR2(NdotH) * smithG_GGX(NdotV) / (4.0f * NdotV);
    }
}
}  // namespace RayTracer
