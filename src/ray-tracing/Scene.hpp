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
#include <optional>
#include <system_error>
#include <thread>
#include <utility>
#include <array>
#include <numbers>
#include "utils/BS_thread_pool.hpp"

namespace RayTracer {

template<u32 window_width, u32 window_height>
class Scene {
    ObjectsList m_objects;
    Camera<window_width, window_height>& m_camera;

public:
    
    Scene(Camera<window_width, window_height>& camera)
        : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;



    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    void per_pixel(u32 x, u32 y, u32 max_bounces, std::array<Vec4<u8>, window_height * window_width>& image) const {
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};

        auto total = Vec4<u32>(0, 0, 0, 0);
        auto hit_count = 0;
        auto multiplier = 1.0f;
        u32 seed = x + y * window_width;
        for (i32 bounce = max_bounces; bounce > 0; bounce--) {
            std::optional<HitPayload> payload = this->m_objects.hit(ray, 0.01f, std::numeric_limits<f32>::max());
            if (payload.has_value()) {
                seed += bounce;
                
                ray.origin = payload->hit_position + Vec3(payload->normal).scale(0.005);
                auto rand_vector = Vec3<f32>::random(seed).normalize();
                if (rand_vector.dot(payload->normal) < 0.0f) {
                    rand_vector = -rand_vector;
                }
                ray.direction = rand_vector;

                hit_count += 1;
                total += payload->object_color.scale(multiplier).cast<u32>();
                multiplier *= 0.5;
            } else {
                hit_count += 1;
                if (hit_count != 1) {
                    multiplier *= 0.3;
                } else {
                    multiplier = 1.f;
                }
                auto a = 0.5 * (ray.direction.y + 1.0);
                total += (Vec4<u32>(255, 255, 255, 255).scale(1.0 - a) + Vec4<u32>(127, 0.7 * 255, 255, 255).scale(a)).scale(multiplier);
                break;
            }

        }
        image[x + y * window_width] = total.scale(1.f / (f32)hit_count).cast<u8>();
    }


    void render(std::array<Vec4<u8>, window_height * window_width>& image) {

        BS::thread_pool thread_pool(8);
        
        for (i32 y = window_height - 1; y >= 0; y--) {
            thread_pool.push_loop(window_width, [this, y, &image](const int a, const int b) {
                for (u32 x = a; x < b; x++) {
                    per_pixel(x, y, 10, image);
                }
            });
        }
        thread_pool.wait_for_tasks();

    }

};  

};
