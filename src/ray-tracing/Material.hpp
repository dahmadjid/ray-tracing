#pragma once
#include <fmt/core.h>

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/ONB.hpp"
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

enum class MaterialType {
    LAMBERTIAN,
    METAL,
    EMISSIVE,

};

struct MaterialParams {
    MaterialType type = MaterialType::LAMBERTIAN; 
    
    Vec3f albedo;
    
    // used in METAL material only
    f32 roughness = 0.5f;

    // used in EMISSIVE material only
    f32 emission_power = 0.0f;
};

class Material : public MaterialParams {
public:
    Material(const MaterialParams& params) : MaterialParams(params) {
        this->alpha = std::max(this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
    }

    void update_roughness(f32 val) {
        val = clamp(val, 0.01f, 1.0f);
        this->roughness = val;
        this->alpha = std::max(this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
        fmt::println("new roughness: {}", this->roughness);
    }

    std::tuple<Vec3f, Vec3f, f32> sample(
        u32& seed, const Vec3f& view_vector, const Vec3f& normal_vector
    ) const {
        f32 r1 = rand_float(seed);
        f32 r2 = rand_float(seed);

        f32 phi = 2 * PI * r2;
        f32 cos_phi = std::cos(phi);
        f32 sin_phi = std::sin(phi);
        if (this->type == MaterialType::LAMBERTIAN) {
            f32 sin_theta = std::sqrt(r1);

            f32 x = cos_phi * sin_theta;
            f32 y = sin_phi * sin_theta;
            f32 z = std::sqrt(1 - r1);
            ONB onb{normal_vector};
            Vec3f light_vector = onb.local(Vec3f(x, y, z));
            f32 pdf = pdf_cosine(z);
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
            Vec3f half_vector =
                onb.local(Vec3f(this->alpha * Nh.x, this->alpha * Nh.y, std::max(0.0f, Nh.z)).normalize());

            Vec3f light_vector = -view_vector.reflect(half_vector);

            f32 NdotH = half_vector.dot(normal_vector);
            f32 VdotH = half_vector.dot(view_vector);
            f32 NdotV = view_vector.dot(normal_vector);
            f32 pdf = pdf_ggx(NdotH, NdotV, VdotH);
            return {light_vector, half_vector, pdf};
        }
    }

    Vec3f brdf(f32 NdotV, f32 NdotH, f32 LdotH, f32 NdotL) const {
        if (NdotL < 0 || NdotV < 0) {
            panic("NdotL = {}, NdotV = {}", NdotL, NdotV);
            return Vec3f();
        };
        if (this->type == MaterialType::LAMBERTIAN) {
            return this->albedo * INV_PI;
        }

        f32 D = D_GGX(NdotH);
        Vec3f F = F_Schlick(LdotH, this->albedo);
        f32 V_SmithGGX = V_SmithGGXCorrelated(NdotV, NdotL);

        return D * V_SmithGGX * F;
    }


    inline Vec3f get_emission() {
        return Vec3(this->albedo).scale(emission_power);
    };

    // https://google.github.io/filament/Filament.html
    inline f32 D_GGX(f32 NoH) const {
        if (NoH < 1e-6) {
            return 0;
        }
        f32 f = (NoH * this->alpha2 - NoH) * NoH + 1.0f;
        return this->alpha2 / ((f32)std::numbers::pi * f * f);
    }

    inline Vec3f F_Schlick(float u, Vec3f f0) const {
        f32 f = std::pow(1.0f - u, 5.0f);
        return f + f0 * (1.0f - f);
    }

    inline f32 V_SmithGGXCorrelated(f32 NoV, f32 NoL) const {
        f32 GGXL = NoV * std::sqrt((-NoL * this->alpha2 + NoL) * NoL + this->alpha2);
        f32 GGXV = NoL * std::sqrt((-NoV * this->alpha2 + NoV) * NoV + this->alpha2);
        return 0.5f / (GGXV + GGXL);
    }

    // NdotL or NdotV
    f32 Smith_G1_GGX(f32 NdotK) const {
        f32 NdotK2 = NdotK * NdotK;
        return 2.0f / (sqrt(((this->alpha2 * (1.0f - NdotK2)) + NdotK2) / NdotK2) + 1.0f);
    }

    inline f32 pdf_cosine(f32 NdotL) const {
        return NdotL * INV_PI;
    }

    inline f32 pdf_ggx(f32 NdotH, f32 NdotV, f32 VdotH) const {
        return D_GGX(NdotH) * Smith_G1_GGX(NdotV) * VdotH / NdotV;
    }

    inline f32 pdf(f32 NdotH, f32 NdotL, f32 NdotV, f32 VdotH) const {
        if (this->type == MaterialType::LAMBERTIAN) {
            return pdf_cosine(NdotL);
        } else {
            return pdf_ggx(NdotH, NdotV, VdotH);
        }
    }


private:
    f32 alpha;
    f32 alpha2;
};
}  // namespace RayTracer