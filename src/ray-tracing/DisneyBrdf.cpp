#include "ray-tracing/DisneyBrdf.hpp"
#include "ray-tracing/objects.hpp"
#include <fmt/core.h>
#include <optional>
#include <cmath>
#include "utils/Panic.hpp"

#define PI (f32)std::numbers::pi

#define CHECK_NAN(number)\
if (std::isnan(number)) {\
    panic("number is nan");\
}\


#define CHECK_NAN_VEC3(vec)\
if (std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z)) {\
    panic("vec is nan {}", vec);\
}\

#define CHECK_ZERO(number)\
if (number == 0) {\
    panic("number is zero");\
}\


#define CHECK_ZERO_VEC3(vec)\
if (vec.x == 0 && vec.y == 0 && vec.z == 0)  {\
    panic("vec is zero {}", vec);\
}\



namespace DisneyBRDF {
static f32 sqr(f32 n) {
    return n*n;
}

static f32 clamp(f32 n, f32 min, f32 max) {
    if (n <= min) {
        return min;
    } else if (n >= max) {
        return max;
    } else {
        return n;
    }
}

static f32 mix(f32 x, f32 y, f32 a) {
    return x * ( 1 - a) + y * a;
}

static Vec3<f32> mix(const Vec3<f32>& x, const Vec3<f32>& y, f32 a) {
    return x * (1 - a) + y * a;
}


f32 SchlickFresnel(f32 u)
{
    f32 m = clamp(1-u, 0, 1);
    f32 m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

f32 GTR1(f32 NdotH, f32 a)
{
    if (a >= 1) {
        return 1.0f / PI;
    }

    f32 a2 = a * a;
    f32 t = 1 + (a2 - 1) * NdotH * NdotH;
    
    return (a2-1) / (PI * std::log(a2) * t);
}

f32 GTR2(f32 NdotH, f32 a)
{
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
    
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom        = PI * denom * denom;
    
	
    return nom / denom;
}

f32 GTR2_aniso(f32 NdotH, f32 HdotX, f32 HdotY, f32 ax, f32 ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

f32 smithG_GGX(f32 NdotV, f32 alphaG)
{
    
    alphaG = std::max(alphaG, 0.04f);
    f32 a = alphaG*alphaG;
    f32 b = NdotV*NdotV;
    return 1 / (NdotV + std::sqrt(a + b - a*b));
}

f32 smithG_GGX_aniso(f32 NdotV, f32 VdotX, f32 VdotY, f32 ax, f32 ay)
{
    return 1 / (NdotV + std::sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

Vec3<f32> mon2lin(const Vec3<f32>& vec)
{
    return Vec3<f32>(std::pow(vec.x, 2.2f), std::pow(vec.y, 2.2f), std::pow(vec.z, 2.2f));
}

static f32 sign(f32 number) {
    if (number < 0) {
        return -1.0f;
    } else {
        return 1.0f;
    }
}
Vec3<f32> BRDF(const Vec3<f32>& L, const Vec3<f32>& V, const Vec3<f32>& N, RayTracer::Material material )
{
    material.roughness = std::max(material.roughness, 0.04f);
    f32 NdotL = N.dot(L);
    f32 NdotV = N.dot(V);
    if (NdotV <= 0) {
        NdotV = -NdotV;
    }
    if (NdotL <= 0) {
        NdotL = -NdotL;
    }

    Vec3<f32> H = (L+V).normalize();
    
    f32 NdotH = N.dot(H);
    f32 LdotH = L.dot(H);

    Vec3<f32> Cdlin = mon2lin(material.albedo);
    f32 Cdlum = 0.3f*Cdlin.x + 0.6f*Cdlin.y + 0.1f*Cdlin.z; // luminance approx.

    Vec3<f32> Ctint = Cdlum > 0 ? Cdlin/Cdlum : Vec3<f32>(1); // normalize lum. to isolate hue+sat
    Vec3<f32> Cspec0 = mix(material.specular * 0.08f * mix(Vec3<f32>(1), Ctint, material.specular_tint), Cdlin, material.metalic);
    Vec3<f32> Csheen = mix(Vec3<f32>(1), Ctint, material.sheen_tint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    f32 FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    f32 Fd90 = 0.5f + 2.0f * LdotH*LdotH * material.roughness;
    f32 Fd = mix(1.0f, Fd90, FL) * mix(1.0f, Fd90, FV);

    // specular
    f32 Ds = GTR1(NdotH, material.roughness);
    f32 FH = SchlickFresnel(LdotH);
    Vec3<f32> Fs = mix(Cspec0, Vec3<f32>(1), FH);
    f32 Gs;
    Gs  = smithG_GGX(NdotL, material.roughness);
    Gs *= smithG_GGX(NdotV, material.roughness);

    // sheen
    Vec3<f32> Fsheen = FH * material.sheen * Csheen;

    Vec3<f32> brdf =((1/PI) * Fd *Cdlin + Fsheen) * (1-material.metalic) + Ds*Gs*Fs;

    return brdf;
}
}

