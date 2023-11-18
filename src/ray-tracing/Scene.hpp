#pragma once

#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/DisneyBrdf.hpp"
#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cmath>
#include <exception>
#include <fmt/core.h>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <system_error>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <array>
#include <numbers>
#include <variant>
#include <vector>
#include "utils/BS_thread_pool.hpp"
#include "utils/Overloaded.hpp"
#include "utils/Panic.hpp"


#define CHECK_NAN(number)\
if (std::isnan(number)) {\
    panic("number is nan");\
}\



namespace RayTracer {

class Scene {
    ObjectsList m_objects;
    LightList m_lights;
public:
    Camera& m_camera;
    
    Scene(Camera& camera) : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;



    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }
    

    template<typename T>
    void add_light(T&& light) {
        m_lights.emplace_back(std::forward<T>(light));
    }

    Vec3<f32> per_pixel(
        u32 x,
        u32 y, 
        u32 max_bounces 
    ) const {
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        Ray spec_ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        auto light = Vec3<f32>(0.0f, 0.0f, 0.0f);
        Vec3<f32> contribution = Vec3<f32>(1.0f);
        Vec3<f32> spec_contribution = Vec3<f32>(1.0f);
        u32 seed = x + y * m_camera.window_width + (m_camera.frame_index << 16);

        std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
        std::optional<HitPayload> spec_payload = payload;
        for (u32 bounce = 0; bounce < max_bounces; bounce++) {
            
            if (!payload.has_value() || !spec_payload.has_value()) {
                light += Vec3<f32>(0.6f, 0.7f, 0.9f) * contribution;
                break;
            }
            // bounce in a random direction ===========================================
            seed += bounce;
            Vec3<f32> rand_vector = Vec3<f32>::random(seed);
            if (payload->normal.dot(rand_vector) < 0) {
                rand_vector = -rand_vector;
            }
            Vec3<f32> view_vector = ray.direction;
            ray.origin = payload->hit_position;
            ray.direction = (payload->normal+rand_vector).normalize();
            Vec3<f32> light_vector = ray.direction;
            light += payload->material.get_emission() * contribution;

            auto brdf = DisneyBRDF::BRDF(light_vector, view_vector, payload->normal, payload->material);

            if (brdf.has_value()) {
                contribution *= (*brdf) * std::abs(light_vector.dot(view_vector)); 
            } else {
                return Vec3<f32>(1.0f, 0, 0);
            }

            {
                  
                Vec3<f32> rand_vector = Vec3<f32>::random(seed);
                if (payload->normal.dot(rand_vector) < 0) {
                    rand_vector = -rand_vector;
                }
    
                Vec3<f32> view_vector = spec_ray.direction;
                spec_ray.origin = spec_payload->hit_position;
                spec_ray.direction = (view_vector.reflect(spec_payload->normal)+rand_vector.scale(1.0f-spec_payload->material.roughness)).normalize();
                Vec3<f32> light_vector = spec_ray.direction;
                light += spec_payload->material.get_emission() * spec_contribution;

                auto brdf = DisneyBRDF::BRDF(light_vector, view_vector, spec_payload->normal, spec_payload->material);

                if (brdf.has_value()) {
                    spec_contribution *= (*brdf) * std::abs(light_vector.dot(view_vector)); 
                } else {
                    return Vec3<f32>(1.0f, 0, 0);
                }
            }            

            payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            spec_payload = this->m_objects.closest_hit(spec_ray, 0.001f, std::numeric_limits<f32>::max());
            
            // if (contribution.x < 0.01) {
            //     contribution.x = 0;
            // }
            // if (contribution.y < 0.01) {
            //     contribution.y = 0;
            // }
            // if (contribution.z < 0.01) {
            //     contribution.z = 0;
            // }
            // // GGX/Throwbridge-reitz NDF ==============================================
            // f32 roughness_squared = payload->material.roughness * payload->material.roughness;
            // f32 denominator_part = NdotH * NdotH * (roughness_squared - 1) + 1;
            // f32 ndf_ggx = roughness_squared / ((f32)std::numbers::pi * denominator_part * denominator_part); 

            // // Smiths GGX Geometry Approximation ======================================
            // f32 roughness_plus_1_squared_div_8 = ((payload->material.roughness + 1) * (payload->material.roughness + 1)) / 8;

            // f32 schlick_ggx_view = NdotV / (NdotV * (1 - roughness_plus_1_squared_div_8) + roughness_plus_1_squared_div_8);
            // f32 schlick_ggx_light = NdotL / (NdotL * (1 - roughness_plus_1_squared_div_8) + roughness_plus_1_squared_div_8);
            // f32 geo_approx_ggx_smiths = schlick_ggx_view * schlick_ggx_light;

            // // Shlick Approximation for fresnel =======================================
            // f32 f0 = (payload->material.ior - 1) * (payload->material.ior - 1) / ((payload->material.ior + 1) * (payload->material.ior + 1));
            // f32 one_minus_VdotH = 1.0f - VdotH;
            // f32 fresnel = f0 + (1.0f - f0) * one_minus_VdotH * one_minus_VdotH *  one_minus_VdotH * one_minus_VdotH * one_minus_VdotH;


            // // Cook-Torrance specular
            // f32 cook_torrance_specular = ndf_ggx * geo_approx_ggx_smiths * fresnel / (4 * NdotV * NdotL);

            // // diffuse: Lambert

            // Vec3<f32> lambert = Vec3(payload->material.albedo).scale(1.0f / (f32)std::numbers::pi).scale(NdotL);
            // Vec3<f32> brdf = lambert.scale((1.0f - fresnel) * (1.0f - payload->material.metalic)) + Vec3(payload->material.albedo).scale(cook_torrance_specular);
             
            // light.shift(cook_torrance_specular);
            // break;


            // light += payload->material.get_emission() * contribution;
            // contribution *= brdf.scale(0.5);
            // contribution *= ;


            // total += payload->material.albedo.cast<f32>().scale(multiplier * light_intensity).cast<u32>();
            // total += Vec4<f32>(payload->normal.x, payload->normal.y, payload->normal.z, 0.f).shift(1.f).scale(1.f/2.f).scale(255).floor<u32>();
            // total = light_count == 0 ? Vec4<u32>(128, 128, 128, 128) : Vec4<u32>(255, 255, 255, 255);

            

            // sample in a random direction
    


        }

        return light;
    }


    void render(u32 max_bounces) {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(8);
        for (i32 y = m_camera.window_height - 1; y >= 0; y--) {
            thread_pool.push_loop(m_camera.window_width, [this, y, max_bounces](const int a, const int b) {
                for (int x = a; x < b; x++) {
                    
                    Vec3<f32> color = per_pixel(x, y, max_bounces);
                    m_camera.accumulation_data[x + y * m_camera.window_width] += color;
                    auto light =  m_camera.accumulation_data[x + y * m_camera.window_width] / (f32)m_camera.frame_index;
                    m_camera.image[x + y * m_camera.window_width] = Vec4<u32>(
                                                                    (u32)(light.x * 255.0f), 
                                                                    (u32)(light.y * 255.0f), 
                                                                    (u32)(light.z * 255.0f), 
                                                                    255
                                                                ).clamp(0, 255).cast<u8>();; 

                }
            });
        }
        thread_pool.wait_for_tasks();
        m_camera.frame_index += 1;

        // std::vector<Vec3<f32>> expose;
        // for (i32 y = m_camera.window_height - 1; y >= 0; y--) {
        //     for (u32 x = 0; x < m_camera.window_width; x++) {
        //         per_pixel(x, y, 1, image, expose);
        //     }
        // }

    }
};  

};
