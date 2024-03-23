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
        Vec3f light{0.0f};
        Ray ray = Ray{.origin = m_camera.position(), .direction = m_camera.get_ray(x, y)};
        Vec3f contribution = Vec3f(1.0f);
        Mesh light_mesh = m_objects.get_object<Mesh>(0);
        std::optional<HitPayload> payload = this->m_objects.closest_hit(ray, 0.001f, std::numeric_limits<f32>::max());
        if (!payload.has_value()) {
            return light;
        }
        if (payload->material.get_emission() != Vec3f(0.0f)) {
            return payload->material.get_emission();
        }

        for (u32 bounce = 0; bounce < max_bounces; ++bounce) {
            Vec3f view_vector = -ray.direction;
            f32 NdotV = payload->normal.dot(view_vector);
            {
                auto [point, pdf] = light_mesh.sample(seed);
                Vec3f light_vector = point - payload->hit_position;
                light_vector.normalize();
                // test if there is nothing blocking the light.
                auto light_sample_payload = this->m_objects.closest_hit(
                    Ray{
                        .origin = payload->hit_position,
                        .direction = light_vector,
                    },
                    0.001f, std::numeric_limits<f32>::max()
                );

                // test if we hit a light.
                if (light_sample_payload.has_value() && light_sample_payload->material.get_emission() != Vec3f()) {
                    f32 NdotL = payload->normal.dot(light_vector);
                    if (NdotL > 0) {
                        Vec3f half_vector = (view_vector + light_vector).normalize();

                        f32 NdotH = payload->normal.dot(half_vector);
                        f32 LdotH = light_vector.dot(half_vector);

                        f32 mis_pdf = (pdf + payload->material.pdf(NdotH, NdotL, NdotV));

                        Vec3f light_contribution =
                            contribution * payload->material.brdf(NdotV, NdotH, LdotH, NdotL) * NdotL / mis_pdf;

                        ray.origin = payload->hit_position;
                        ray.direction = light_vector;

                        light += light_sample_payload->material.get_emission() * light_contribution;
                    }
                }
            }
            {
                auto [light_vector, half_vector, pdf] = payload->material.sample(seed, view_vector, payload->normal);
                f32 NdotL = payload->normal.dot(light_vector);
                if (NdotL <= 0) {
                    break;
                }
                f32 NdotH = payload->normal.dot(half_vector);
                f32 LdotH = light_vector.dot(half_vector);

                f32 mis_pdf = (pdf + light_mesh.pdf(light_vector, payload->hit_position, payload->normal));

                contribution *= payload->material.brdf(NdotV, NdotH, LdotH, NdotL) * NdotL / mis_pdf;

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
                    auto light = m_camera.accumulation_data[x + y * m_camera.window_width] / (f32)m_camera.frame_index;
                    m_camera.image[x + y * m_camera.window_width] =
                        Vec4<u32>(
                            (u32)(std::pow(light.x, 1.0f / 2.2f) * 255.0f),
                            (u32)(std::pow(light.y, 1.0f / 2.2f) * 255.0f),
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
