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

public:
    
    Scene(Camera<window_width, window_height>& camera)
        : m_camera(camera) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;

    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    void render(std::array<Vec4<uint8_t>, window_height * window_width>& image) {
        for (int y = window_height - 1; y >= 0; y--) {
            for (uint32_t x = 0; x < window_width; x++) {
                Ray ray = Ray{.origin=m_camera.position(), .direction=m_camera.ray_directions[x + y * window_width]};

                auto total = Vec4<float>(0, 0, 0, 0);
                auto hit_count = 0;
                auto multiplier = 0.7f;
                uint32_t seed = x + y * window_width;
                int bounces = 10;
                for (int bounce = bounces; bounce > 0; bounce--) {
                    std::optional<HitPayload> payload = this->m_objects.hit(ray, 0.01f, std::numeric_limits<float>::max());
                    if (payload.has_value()) {
                        seed += bounce;
                        
                        ray.origin = payload->hit_position + Vec3(payload->normal).scale(0.005);
                        auto rand_vector = Vec3<float>::random(seed).normalize();
                        if (rand_vector.dot(payload->normal) < 0.0f) {
                            rand_vector = -rand_vector;
                        }
                        ray.direction = rand_vector;

                        hit_count += 1;
                        multiplier *= 0.7;
                        
                    } else {
                        auto a = 0.5 * (ray.direction.y + 1.0);
                        total = total + Vec4<float>(1.0, 1.0, 1.0, 1.0).scale(1.0 - a) + Vec4<float>(0.5, 0.7, 1.0, 1.0).scale(a);
                        total = total.scale(multiplier);
                        break;
                    }

                }
                if (hit_count > 0) {
                    image[x + y * window_width] = total.scale(1. / static_cast<float>(hit_count)).scale(255.99).floor<uint8_t>();

                } else {
                    auto a = 0.5 * (ray.direction.y + 1.0);

                    total = Vec4<float>(1.0, 1.0, 1.0, 1.0).scale(1.0 - a) + Vec4<float>(0.5, 0.7, 1.0, 1.0).scale(a);
                    image[x + y * window_width] = total.scale(255.99).floor<uint8_t>();
                }

            }
        }

    }

};  

};
