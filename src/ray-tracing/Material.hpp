#pragma once
#include <fmt/core.h>

#include "linear_algebra/Vec3.hpp"
#include "utils/MathUtils.hpp"

#define CHECK_NAN_VEC3(vec)                                            \
    if (std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z)) { \
        panic("vec is nan {}", vec);                                   \
    }

#define CHECK_NEG_VEC3(vec)                    \
    if (vec.x < 0 || vec.y < 0 || vec.z < 0) { \
        panic("vec is neg {}", vec);           \
    }

namespace RayTracer {

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
    f32 metallic = 0.0f;
    f32 subsurface = 0.0f;
    f32 specular = 0.5f;
    f32 roughness = 0.5f;
    f32 specularTint = 0.0f;
    f32 anisotropic = 0.0f;
    f32 sheen = 0.0f;
    f32 sheenTint = 0.0f;
    f32 cleaercoat = 0.0f;
    f32 clearcoatGloss = 0.0f;
    f32 emission_power = 0.0f;
};

class Material : public MaterialParams {
public:
    Material(const MaterialParams& params) : MaterialParams(params) {
        this->alpha = std::max(this->roughness * this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
    }

    void update_roughness(f32 val) {
        val = clamp(val, 0.001f, 1.0f);
        this->roughness = val;
        this->alpha = std::max(this->roughness * this->roughness, 0.001f);
        this->alpha2 = this->alpha * this->alpha;
        fmt::println("new roughness: {}", this->roughness);
    }

    Vec3f brdf(f32 NdotV, f32 NdotH, f32 LdotH, f32 NdotL) const;
    std::tuple<Vec3f, Vec3f, f32> sample(u32& seed, const Vec3f& view_vector, const Vec3f& normal_vector) const;

    f32 pdf(f32 NdotH, f32 NdotL, f32 VdotH);

    inline Vec3f get_emission() {
        return Vec3(this->albedo).scale(emission_power);
    };

    inline f32 SchlickFresnel(f32 u) const {
        f32 m = clamp(1 - u, 0, 1);
        f32 m2 = m * m;
        return m2 * m2 * m;  // pow(m,5)
    }

    inline f32 GTR1(f32 NdotH) const {
        f32 t = (1 + (this->alpha2 - 1.0f) * NdotH * NdotH);
        return (this->alpha2 - 1.0f) / (PI * std::log(this->alpha2) * t);
    }

    inline f32 GTR2(f32 NdotH) const {
        f32 t = (1 + (this->alpha2 - 1.0f) * NdotH * NdotH);
        return (this->alpha2) / (PI * t * t);
    }

    f32 smithG_GGX(f32 NdotV) const {
        f32 b = NdotV * NdotV;
        return 1 / (NdotV + std::sqrt(this->alpha2 + b - this->alpha2 * b));
    }

    inline Vec3f mon2lin(const Vec3f& v) const {
        return Vec3f(std::pow(v.x, 2.2f), std::pow(v.y, 2.2f), std::pow(v.z, 2.2f));
    }

private:
    f32 alpha;
    f32 alpha2;
};
}  // namespace RayTracer