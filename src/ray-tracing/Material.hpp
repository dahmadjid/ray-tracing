#pragma once

#include <linear_algebra/Vec3.hpp>
#include <optional>
namespace RayTracer {
// ::begin parameters
// color baseColor .82 .67 .16
// float metallic 0 1 0
// float subsurface 0 1 0
// float specular 0 1 .5
// float roughness 0 1 .5
// float specularTint 0 1 0
// float anisotropic 0 1 0
// float sheen 0 1 0
// float sheenTint 0 1 .5
// float clearcoat 0 1 0
// float clearcoatGloss 0 1 1
// ::end parameters


struct MaterialParams {
    Vec3f albedo;
    f32 roughness = 0.5f;
    f32 metallic = 0.0f;
    f32 emission_power = 0.0f;
    f32 ior = 1.5f;
    f32 subsurface = 0.0f;
    f32 specular = 0.5f;
    f32 specular_tint = 0.0f;
    f32 anisotropic = 0.0f;
    f32 sheen = 0.0f;
    f32 sheen_tint = 0.5f;
    f32 clearcoat = 0.0f;
    f32 clearcoat_gloss = 1.0f;
};

class Material: public MaterialParams {
public:
    Material(const MaterialParams& params): MaterialParams(params) {
        f32 alpha2 = this->roughness * this->roughness;
        two_alpha_two = 2 * alpha2;
        alpha_two_minus_one = alpha2 - 1;
    }

    


    std::tuple<Vec3f, f32> sample(u32& seed, const Vec3f& V) const;
    Vec3f brdf(const Vec3f& L, const Vec3f& V, const Vec3f& N, const Vec3f& H) const;
    inline Vec3f get_emission() {
        return Vec3(this->albedo).scale(emission_power);
    };
private:
    f32 pdf(f32 cos_theta, f32 sin_theta) const;
    f32 two_alpha_two;
    f32 alpha_two_minus_one;

};
}