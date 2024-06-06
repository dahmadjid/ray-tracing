#pragma once

#include <fmt/core.h>
#include <math.h>

#include <array>
#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <optional>
#include <system_error>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Material.hpp"
#include "ray-tracing/Ray.hpp"
#include "ray-tracing/objects.hpp"
#include "utils/BS_thread_pool.hpp"
#include "utils/Panic.hpp"
#include "utils/types.hpp"
#include <rfl/json.hpp>
#define CHECK_NAN(number)       \
    if (std::isnan(number)) {   \
        panic("number is nan"); \
    }



namespace RayTracer {

class Scene {
    ObjectsList m_objects;

public:
    Camera& m_camera;

    Scene(Camera& camera) : m_camera(camera) {}

    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;

    template <typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward<T>(hittable_object));
    }

    template <typename T>
    inline T& get_object(u32 index) {
        return m_objects.get_object<T>(index);
    }

    Vec3f per_pixel(u32 x, u32 y, u32 max_bounces) const {
        u32 seed = x + y * m_camera.window_width + (m_camera.frame_index << 16);
        f32 _x = rand_float(seed) - 0.5f;
        f32 _y = rand_float(seed) - 0.5f;
        Vec3f light{0.0f};
        Ray ray =
            Ray{.origin = m_camera.position(),
                .direction = m_camera.get_ray(x, y) + Vec3f(_x * m_camera.pixel_delta_u, _y * m_camera.pixel_delta_v, 0)
            };
        Vec3f contribution = Vec3f(1.0f);
        Mesh light_mesh = m_objects.get_object<Mesh>(0);
        std::optional<HitPayload> payload =
            this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
        
        if (!payload.has_value()) {
            return light;
        }

        if (payload->material.get_emission() != Vec3f(0.0f)) {
            return payload->material.get_emission();
        }

        u32 bounce = 0;
        for (bounce = 0; bounce < max_bounces; ++bounce) {
            Vec3f view_vector = -ray.direction;
            Vec3f random_vector = Vec3f::random(seed);
            if (random_vector.dot(payload->normal) < 0) {
                random_vector = -random_vector;
            }
            Vec3f light_vector = random_vector.normalize();
            f32 NdotL = payload->normal.dot(light_vector);
            if (NdotL <= 0) {
                break;
            }
            contribution *= payload->material.albedo * INV_PI * NdotL;
            ray.origin = payload->hit_position;
            ray.direction = light_vector;

            payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
            if (!payload.has_value()) {
                break;
            }
            light += payload->material.get_emission() * contribution;
            if (payload->material.get_emission() != Vec3f(0.0f)) {
                break;
            }
        }
        return light;
    }

    void render(u32 max_bounces) {
        this->m_camera.calculate_ray_directions();
        BS::thread_pool thread_pool(8);
        for (i32 y = m_camera.window_height - 1; y >= 0; --y) {
            thread_pool.push_loop(m_camera.window_width, [this, y, max_bounces](const int a, const int b) {
                for (int x = a; x < b; ++x) {
                    Vec3f color = per_pixel(x, y, max_bounces);
                    m_camera.accumulation_data[x + y * m_camera.window_width] += color;
                    auto light = m_camera.accumulation_data[x + y * m_camera.window_width] / (f32)m_camera.frame_index;
                    m_camera.image[x + y * m_camera.window_width] =
                        Vec4<u32>(
                            (u32)(std::pow(light.x, 1.0f / 2.2f) * 255.0f), (u32)(std::pow(light.y, 1.0f / 2.2f) * 255.0f),
                            (u32)(std::pow(light.z, 1.0f / 2.2f) * 255.0f), 255
                        )
                            .clamp(0, 255)
                            .cast<u8>();
                    ;
                }
            });
        }
        thread_pool.wait_for_tasks();
        m_camera.frame_index += 1;
    }
};

};  // namespace RayTracer
