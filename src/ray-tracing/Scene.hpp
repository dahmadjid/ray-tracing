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
    inline T& get_object(u32 index) {
        return m_objects.get_object<T>(index);
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
        u32 seed = x + y * m_camera.window_width + (m_camera.frame_index << 16);
        auto light = Vec3<f32>(0.0f, 0.0f, 0.0f);
        auto spec_light = Vec3<f32>(0.0f, 0.0f, 0.0f);

        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        Vec3<f32> contribution = Vec3<f32>(1.0f);
        std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
        for (u32 bounce = 0; bounce < max_bounces; bounce++) {
            if (!payload.has_value()) {
                // light += Vec3<f32>(0.6f, 0.7f, 0.9f) * contribution;
                break;
            }
            seed += bounce;
            Vec3<f32> rand_vector = Vec3<f32>::random(seed);
            if (payload->normal.dot(rand_vector) < 0) {
                rand_vector = -rand_vector;
            }
            Vec3<f32> view_vector = ray.direction;
            ray.origin = payload->hit_position;
            ray.direction = rand_vector.normalize();
            Vec3<f32> light_vector = ray.direction;
            light += payload->material.get_emission() * contribution;

            contribution *= DisneyBRDF::BRDF(light_vector, view_vector, payload->normal, payload->material) * payload->normal.dot(light_vector);

            payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
        }


        {
            Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
            auto& light = spec_light;
            Vec3<f32> contribution = Vec3<f32>(1.0f);
            std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            for (u32 bounce = 0; bounce < max_bounces; bounce++) {
                if (!payload.has_value()) {
                    // light += Vec3<f32>(0.6f, 0.7f, 0.9f) * contribution;
                    break;
                }
                Vec3<f32> rand_vector = Vec3<f32>::random(seed);
                if (payload->normal.dot(rand_vector) < 0) {
                    rand_vector = -rand_vector;
                }
                Vec3<f32> view_vector = ray.direction;
                ray.origin = payload->hit_position;
                ray.direction = (view_vector.reflect(payload->normal) + rand_vector.scale(payload->material.roughness)).normalize();
                Vec3<f32> light_vector = ray.direction;
                light += payload->material.get_emission() * contribution;

                contribution *= DisneyBRDF::BRDF(light_vector, view_vector, payload->normal, payload->material) * payload->normal.dot(light_vector);


                payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            }
        }

        return (spec_light + light) / 2.0f ;
    }


    void render(u32 max_bounces) {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(16);
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
