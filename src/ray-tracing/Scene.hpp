#pragma once

#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <limits>
#include <optional>
#include <utility>
#include <array>
#include <numbers>

namespace RayTracer {

template<uint32_t window_width, uint32_t window_height>
class Scene {
    ObjectsList m_objects;
    Camera<window_width, window_height>& m_camera;
    const uint32_t m_window_width;
    const uint32_t m_window_height;


public:
    
    Scene(Camera<window_width, window_height>& camera)
        : m_camera(camera), m_window_width(window_width), m_window_height(window_height) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;

    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    std::array<Vec4<uint8_t>, window_height * window_width> render() {

        std::array<Vec4<uint8_t>, window_height * window_width> image;
        for (int y = m_window_height - 1; y >= 0; y--) {
            for (uint32_t x = 0; x < m_window_width; x++) {
                Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.ray_directions()[x + y * window_width]};
                auto total = Vec4<float>(0, 0, 0, 0);
                auto hit_count = 0;
                auto light_dir = Vec3<float>(-1., -1., -1.).normalize();
                auto multiplier = 1.0f;
                int bounces = 2;
                for (int bounce = bounces; bounce > 0; bounce--) {
                    std::optional<HitPayload> payload = this->m_objects.hit(ray, 0.f, 1);
                    if (false) {
                        auto v = Vec4<float>::random();
                        while (true) {
                            if (v.length_squared() < 1.0) {
                                break;
                            } 
                            v = Vec4<float>::random();
                        };
                        
                        ray.origin = payload->hit_position + Vec3(payload->normal).scale(0.005);
                        ray.direction = payload->normal;
                        total = total + payload->object_color.cast<float>().scale(std::max(payload->normal.dot(-light_dir), 0.0f) * multiplier);
                        hit_count += 1;
                        multiplier *= 0.7;
                        
                    } else {
                        total = total + Vec4<float>(135./255., 206./255., 235./255., 1).scale(0.05);
                        break;
                    }

                }
                if (hit_count > 0) {
                    image[x + y * m_window_width] = total.scale(1. / static_cast<float>(hit_count)).scale(255.99).floor<uint8_t>();

                } else {
                    image[x + y * m_window_width] = Vec4<uint8_t>(135, 206, 235, 255);
                }

            }
        }

        return image;
        
    }

};  

};
