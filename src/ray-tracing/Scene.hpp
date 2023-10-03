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
    LightList m_lights;


public:
    Camera<window_width, window_height>& m_camera;
    
    Scene(Camera<window_width, window_height>& camera) : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;



    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    template<typename T>
    void add_light(T&& light_object) {
        m_lights.emplace_back(std::forward<T>(light_object));
    }

    Vec4<u8> per_pixel(
        u32 x,
        u32 y, 
        u32 max_bounces 
    ) const {
        
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        auto total = Vec4<u32>(0, 0, 0, 0);
        auto hit_count = 0;
        auto multiplier = 1.f;
        u32 seed = x + y * window_width + (m_camera.frame_index << 16);

        for (u32 bounce = 0; bounce < max_bounces; bounce++) {
            std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            if (payload.has_value()) {
                seed += bounce;
                ray.origin = payload->hit_position;

                f32 light_intensity = 0;
                u32 light_count = 0;
                
                for (const auto& light: m_lights) {
                    auto ret = std::visit(overloaded {
                        [&](const PointLight& light) -> f32 {
                            Ray shadow_ray;
                            shadow_ray.origin = payload->hit_position;
                            shadow_ray.direction = (light.position - shadow_ray.origin);
                            auto p = m_objects.any_hit(shadow_ray, 0.001f, std::numeric_limits<f32>::max());               
                            if (p.has_value()) {
                                return -1.f;
                            } else {
                                return std::max(payload->normal.dot(shadow_ray.direction), 0.0f);  // == cos(angle)
                            }
                        }
                    }, light);

                    if (ret > 0) {
                        light_intensity += ret;
                        light_count += 1;
                    }
                }
                
                if (light_count > 0) {
                    light_intensity /= (f32)light_count;
                }


                Vec3<f32> rand_vector = Vec3<f32>::random(seed).normalize();
                ray.direction = ray.direction.reflect(payload->normal) + rand_vector.scale(payload->material.roughness / 2.0f);
                hit_count += 1;
                total += payload->material.albedo.cast<f32>().scale(multiplier * std::max(light_intensity, 0.5f)).cast<u32>();
                // total += Vec4<f32>(payload->normal.x, payload->normal.y, payload->normal.z, 0.f).shift(1.f).scale(1.f/2.f).scale(255).floor<u32>();
                // total = light_count == 0 ? Vec4<u32>(128, 128, 128, 128) : Vec4<u32>(255, 255, 255, 255);
                multiplier *= std::max(0.5f - payload->material.roughness / 2.0f, 0.0f);
            } else {
                if (hit_count != 0) {
                    total += Vec4<f32>(255).scale(multiplier * 0.5f).cast<u32>();
                } else {
                    total += Vec4<u32>(200, 200, 240, 255);
                }
                // auto a = 0.5 * ((f32)ray.direction.y + 1.0);
                // total += (Vec4<u32>(255, 255, 255, 255).scale(1.0 - a) + Vec4<u32>(127, 0.9 * 255, 255, 255).scale(a)).scale(multiplier);
                break;
            }

        }
        
        return total.clamp(0, 255).cast<u8>();
    }


    void render() {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(8);
        for (i32 y = window_height - 1; y >= 0; y--) {
            thread_pool.push_loop(window_width, [this, y](const int a, const int b) {
                for (int x = a; x < b; x++) {
                    Vec4<u8> color = per_pixel(x, y, 10);
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
