#pragma once
#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/objects.hpp"
#include <optional>

// Credits: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf

namespace BRDF {
float SchlickFresnel(float u);

float GTR1(float NdotH, float a);

float GTR2(float NdotH, float a);

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay);

float smithG_GGX(float NdotV, float alphaG);

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay);

Vec3<f32> mon2lin(const Vec3<f32>& albedo);

Vec3<f32> BRDF(const Vec3<f32>& L, const Vec3<f32>& V, const Vec3<f32>& N, RayTracer::Material material);


}