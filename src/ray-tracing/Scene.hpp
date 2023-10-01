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
#include <optional>
#include <system_error>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <array>
#include <numbers>
#include <vector>
#include "utils/BS_thread_pool.hpp"

namespace RayTracer {

template<u32 window_width, u32 window_height>
class Scene {

public:
    ObjectsList m_objects;
    LightList m_lights;
    Camera<window_width, window_height>& m_camera;
    
    Scene(Camera<window_width, window_height>& camera)
        : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;



    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    template<typename T>
    void add_light(T&& light_object) {
        m_lights.add_object(std::forward<T>(light_object));
    }

    void per_pixel(u32 x, u32 y, u32 max_bounces, std::array<Vec4<u8>, window_height * window_width>& image, std::unordered_map<std::string, int>& expose, bool expose_) const {
        Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.get_ray(x, y)};
        auto total = Vec4<u32>(0, 0, 0, 0);
        auto hit_count = 0;
        auto multiplier = 1.f;
        u32 seed = x + y * window_width;
        

        for (i32 bounce = max_bounces; bounce > 0; bounce--) {
            std::optional<HitPayload> payload = this->m_objects.hit(ray, 0.01f, std::numeric_limits<f32>::max());
            if (payload.has_value()) {
                seed += bounce;
                f32 light_factor = 1.0f;
                ray.origin = payload->hit_position + Vec3(payload->normal).scale(0.01);

                m_lights.for_each(
                    [&](const Hittable auto& light){
                        Ray shadow_ray;
                        shadow_ray.origin=ray.origin;
                        shadow_ray.direction = light.position() - shadow_ray.origin;
                        auto p = m_objects.hit(shadow_ray, 0.01f, std::numeric_limits<f32>::max());               
                        if (p.has_value()) {
                            light_factor = 0.5;
                            if (expose_) {
                                expose[fmt::format("{}",p->hit_position)] = 0;
                            }
                        }
                    }
                );
                auto rand_vector = Vec3<f32>::random(seed).normalize();
                // if (rand_vector.dot(payload->normal) < 0.0f) {
                //     rand_vector = -rand_vector;
                // }
                ray.direction = payload->normal;
                hit_count += 1;
                // total += payload->object_color.scale(multiplier * light_factor).cast<u32>();
                
                // total += Vec4<f32>(payload->normal.x, payload->normal.y, payload->normal.z, 0.f).shift(1.f).scale(1.f/2.f).scale(255).floor<u32>();
                total = light_factor == 0.5 ? Vec4<u32>(0, 0, 0, 0) : Vec4<u32>(255, 255, 255, 255);

                if (!expose_ && expose.find(fmt::format("", payload->hit_position)) != expose.end()) {
                    total = Vec4<u32>(255, 0, 0, 255);
                    fmt::println("exposed ?");
                }

                
                break;
                multiplier *= 0.3;
            } else {
                hit_count += 1;
                if (hit_count != 1) {
                    multiplier *= 0.3;
                } else {
                    multiplier = 1.f;
                }
                auto a = 0.5 * ((f32)ray.direction.y + 1.0);

                total += (Vec4<u32>(255, 255, 255, 255).scale(1.0 - a) + Vec4<u32>(127, 0.9 * 255, 255, 255).scale(a)).scale(multiplier);
                
                break;
            }

        }
        
        image[x + y * window_width] = total.clamp(0, 255).cast<u8>();




    }


    void render(std::array<Vec4<u8>, window_height * window_width>& image) {

        // BS::thread_pool thread_pool(8);
        // for (i32 y = window_height - 1; y >= 0; y--) {
        //     thread_pool.push_loop(window_width, [this, y, &image](const int a, const int b) {
        //         for (u32 x = a; x < b; x++) {
        //             per_pixel(x, y, 2, image);
        //         }
        //     });
        // }
        // thread_pool.wait_for_tasks();
        std::unordered_map<std::string, int> expose;
        for (i32 y = window_height - 1; y >= 0; y--) {
            for (u32 x = 0; x < window_width; x++) {
                per_pixel(x, y, 2, image, expose, true);
            }
        }

        for (i32 y = window_height - 1; y >= 0; y--) {
            for (u32 x = 0; x < window_width; x++) {
                per_pixel(x, y, 2, image, expose, false);
            }
        }


    }
};  

};
