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

Vec3f mon2lin(const Vec3f& albedo);

Vec3f BRDF(const Vec3f& L, const Vec3f& V, const Vec3f& N, const Vec3f& H, const RayTracer::Material material);


}