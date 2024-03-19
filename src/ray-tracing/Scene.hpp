#pragma once

#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cmath>
#include <exception>
#include <fmt/core.h>
#include <functional>
#include <limits>
#include <map>
#include <math.h>
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
#include "utils/MathUtils.hpp"
#include "utils/Overloaded.hpp"
#include "utils/Panic.hpp"
#include "utils/types.hpp"


#define CHECK_NAN(number)\
if (std::isnan(number)) {\
    panic("number is nan");\
}\



namespace RayTracer {

class Scene {
    ObjectsList m_objects;
    
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
    inline T& get_object(u32 index) {
        return m_objects.get_object<T>(index);
    }

    Vec3f per_pixel(
        u32 x,
        u32 y, 
        u32 max_bounces 
    ) const {
        u32 seed = x + y * m_camera.window_width + (m_camera.frame_index << 16);
        Vec3f light{ 0.0f };
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        Vec3f contribution = Vec3f(1.0f);
        Mesh light_mesh = m_objects.get_object<Mesh>(0); 

        for (u32 bounce = 0; bounce < max_bounces; ++bounce) {
            std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            if (!payload.has_value()) {
                // light += Vec3f(0.6f, 0.7f, 0.9f) * contribution;
                break;
            }
            Vec3f view_vector = -ray.direction;
            
            Vec3f light_vector;
            f32 pdf;

            if (rand_float(seed) > 0.5f) {
                Vec3f rand_vector = Vec3f::random(seed);
                if (rand_vector.dot(payload->normal) < 0) {
                    rand_vector = -rand_vector;
                }
                light_vector = rand_vector.normalize();
                // auto sample = payload->material.sample(seed, view_vector, payload->normal);
                // light_vector = std::get<1>(sample);
                // pdf = std::get<2>(sample);
                pdf = 0.5f + 0.5f * light_mesh.pdf(light_vector, payload->hit_position, payload->normal);
            } else {
                auto sample = light_mesh.sample(seed);
                light_vector = sample.first - payload->hit_position;
                pdf = 0.5f * sample.second + 0.5f;
            }
            if (pdf < 0.001) {
                return Vec3f(0.0, 0.0, 0.0);
            }
            // Vec3f light_vector;
            // Vec3f rand_vector = Vec3f::random(seed);
            // // if (rand_float(seed) > 0.95f) {
            // //     light_vector = (ray.direction.reflect(payload->normal) + rand_vector.scale(payload->material.roughness)).normalize();
            // // } else {
            // if (rand_vector.dot(payload->normal) < 0) {
            //     rand_vector = -rand_vector;
            // }
            // light_vector = rand_vector.normalize();
            // // }
            // Vec3f half_vector = (view_vector + light_vector).normalize();
            // f32 pdf = 1.0f;
            // // }

            f32 NdotV = std::abs(payload->normal.dot(view_vector)) + 1e-5f;
            f32 NdotL = clamp(payload->normal.dot(light_vector), 0, 1);
            f32 NdotH = clamp(payload->normal.dot(Vec3f(0.2f)), 0, 1);
            f32 LdotH = clamp(light_vector.dot(Vec3f(0.2f)), 0, 1);

            light += payload->material.get_emission() * contribution;

            contribution *= payload->material.brdf(NdotV, NdotH, LdotH, NdotL) * NdotL / pdf;
            ray.origin = payload->hit_position;
            ray.direction = light_vector;
        }
        return light;
    }


    void render(u32 max_bounces) {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(8);
        for (i32 y = m_camera.window_height - 1; y >= 0; y--) {
            thread_pool.push_loop(m_camera.window_width, [this, y, max_bounces](const int a, const int b) {
                for (int x = a; x < b; x++) {
                    
                    Vec3f color = per_pixel(x, y, max_bounces);
                    m_camera.accumulation_data[x + y * m_camera.window_width] += color;
                    auto light =  m_camera.accumulation_data[x + y * m_camera.window_width] / (f32)m_camera.frame_index;
                    m_camera.image[x + y * m_camera.window_width] = Vec4<u32>(
                                                                    (u32)(std::sqrt(light.x) * 255.0f), 
                                                                    (u32)(std::sqrt(light.y) * 255.0f), 
                                                                    (u32)(std::sqrt(light.z) * 255.0f), 
                                                                    255
                                                                ).clamp(0, 255).cast<u8>();; 

                }
            });
        }
        thread_pool.wait_for_tasks();
        m_camera.frame_index += 1;
    }
};  

};
