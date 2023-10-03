#pragma once

#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
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

namespace RayTracer {

template<u32 window_width, u32 window_height>
class Scene {
    ObjectsList m_objects;

public:
    Camera<window_width, window_height>& m_camera;
    
    Scene(Camera<window_width, window_height>& camera) : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;



    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    Vec4<u8> per_pixel(
        u32 x,
        u32 y, 
        u32 max_bounces 
    ) const {
        
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        auto light = Vec3<f32>(0, 0, 0);
        Vec3<f32> contribution = Vec3<f32>(1.0f);
        u32 seed = x + y * window_width + (m_camera.frame_index << 16);

        for (u32 bounce = 0; bounce < max_bounces; bounce++) {
            std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            
            if (!payload.has_value()) {
                // light += Vec3<f32>(0.6f, 0.7f, 0.9f) * contribution;
                break;
            }

            seed += bounce;
            Vec3<f32> rand_vector = Vec3<f32>::random(seed);
            if (ray.direction.dot(rand_vector) < 0) {
                rand_vector = -rand_vector;
            }
            ray.origin = payload->hit_position;
            light += payload->material.get_emission() * contribution;
            contribution *= payload->material.albedo;
            ray.direction = (payload->normal + rand_vector).normalize();


            // total += payload->material.albedo.cast<f32>().scale(multiplier * light_intensity).cast<u32>();
            // total += Vec4<f32>(payload->normal.x, payload->normal.y, payload->normal.z, 0.f).shift(1.f).scale(1.f/2.f).scale(255).floor<u32>();
            // total = light_count == 0 ? Vec4<u32>(128, 128, 128, 128) : Vec4<u32>(255, 255, 255, 255);

        }
        return Vec4<u32>(
            (u32)(light.x * 255.0f), 
            (u32)(light.y * 255.0f), 
            (u32)(light.z * 255.0f), 
            255
        ).clamp(0, 255).cast<u8>();
    }


    void render(u32 max_bounces) {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(8);
        for (i32 y = window_height - 1; y >= 0; y--) {
            thread_pool.push_loop(window_width, [this, y, max_bounces](const int a, const int b) {
                for (int x = a; x < b; x++) {
                    Vec4<u8> color = per_pixel(x, y, max_bounces);
                    m_camera.accumulation_data->operator[](x + y * window_width) += color.cast<u32>();
                    Vec4<u32> accumulated_color = m_camera.accumulation_data->operator[](x + y * window_width);
                    accumulated_color = (accumulated_color.cast<f32>().scale(1.f/static_cast<f32>(m_camera.frame_index))).clamp(0, 255).cast<u32>();
                    m_camera.image->operator[](x + y * window_width) = accumulated_color.cast<u8>(); 
                }
            });
        }
        thread_pool.wait_for_tasks();
        m_camera.frame_index += 1;

        // std::vector<Vec3<f32>> expose;
        // for (i32 y = window_height - 1; y >= 0; y--) {
        //     for (u32 x = 0; x < window_width; x++) {
        //         per_pixel(x, y, 1, image, expose);
        //     }
        // }

    }
};  

};
