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
    f32 a2 = a*a;
    f32 t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

f32 GTR2_aniso(f32 NdotH, f32 HdotX, f32 HdotY, f32 ax, f32 ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

f32 smithG_GGX(f32 NdotV, f32 alphaG)
{
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
std::optional<Vec3<f32>> BRDF(const Vec3<f32>& L, const Vec3<f32>& V, const Vec3<f32>& N, const RayTracer::Material& material )
{
    float s = sign(N.z);
    float a = -1.0f / (s + N.z);
    float b = N.x * N.y * a;

    // two arbitrary, orthonormal vectors for the tangent and bitangent
    Vec3<f32> X = Vec3<f32>(1 + s * sqr(N.x) * a, s * b, -s * N.x);
    Vec3<f32> Y = Vec3<f32>(b, s + sqr(N.y) * a, -N.y);
    
    f32 NdotL = N.dot(L);
    f32 NdotV = N.dot(V);
    if (NdotV <= 0) {
        NdotV = -NdotV;
    }
    if (NdotL <= 0 || NdotV <= 0) {
        return std::nullopt;
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

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    f32 Fss90 = LdotH*LdotH*material.roughness;
    f32 Fss = mix(1.0f, Fss90, FL) * mix(1.0f, Fss90, FV);
    f32 ss = 1.25f * (Fss * (1.0f / (NdotL + NdotV) - 0.5f) + 0.5f);

    // specular
    f32 aspect = (f32)std::sqrt(1.0f - material.anisotropic * 0.9f);
    f32 ax = std::max(.001f, sqr(material.roughness)/aspect);
    f32 ay = std::max(.001f, sqr(material.roughness)*aspect);
    f32 Ds = GTR2_aniso(NdotH, H.dot(X), H.dot(Y), ax, ay);
    f32 FH = SchlickFresnel(LdotH);
    Vec3<f32> Fs = mix(Cspec0, Vec3<f32>(1), FH);
    f32 Gs;
    Gs  = smithG_GGX_aniso(NdotL, L.dot(X), L.dot(Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, V.dot(X), V.dot(Y), ax, ay);

    // sheen
    Vec3<f32> Fsheen = FH * material.sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    f32 Dr = GTR1(NdotH, mix(0.1f, .001f, material.clearcoat_gloss));
    f32 Fr = mix(0.04f, 1.0f, FH);
    f32 Gr = smithG_GGX(NdotL, .25f) * smithG_GGX(NdotV, .25f);

    Vec3<f32> brdf = ((1/PI) * mix(Fd, ss, material.subsurface)*Cdlin + Fsheen)
        * (1-material.metalic)
        + Ds*Gs*Fs + .25f*material.clearcoat*Gr*Fr*Dr;

    CHECK_NAN_VEC3(brdf);
    return brdf;
}
}

