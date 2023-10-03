#pragma once

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/SameAsAny.hpp"
#include <algorithm>
#include <concepts>
#include <fmt/core.h>
#include <optional>
#include <vector>
#include <variant>
#include "utils/Overloaded.hpp"


namespace RayTracer {
struct HitPayload {
    Vec3<f32> hit_position;
    Vec3<f32> normal;
    f32 t = 0;
    bool front_face = false;
    Vec4<u8> object_color;
};

template<typename T>
concept Hittable = requires (T& object, const Vec3<f32>& position, const Ray& ray, f32 t_min, f32 t_max){
    { object.position() } -> std::same_as<Vec3<f32>>;
    { object.set_position(position) } -> std::same_as<void>;
    { object.hit(ray, t_min, t_max) } -> std::same_as<std::optional<HitPayload>>;
    { object.object_color() } -> std::same_as<Vec4<u8>>;
};



template<Hittable...Ts>
class HittableList {
public:
    template<same_as_any<Ts...>  T>
    void add_object(T&& hittable_object) {
        m_hittable_objects.emplace_back(std::forward<T>(hittable_object));
    }
    
    std::optional<HitPayload> closest_hit(const Ray& ray, f32 t_min, f32 t_max) const {
        std::optional<HitPayload> closest_payload = std::nullopt;
        for (const auto& object: m_hittable_objects) {

            std::visit(overloaded {
                [&](const auto& object) {
                    std::optional<HitPayload> payload = object.hit(ray, t_min, t_max);
                    if (payload.has_value()) {
                        if (closest_payload.has_value()) {
                            if (closest_payload->t >= payload->t) {
                                closest_payload = payload;
                            }
                        } else {
                            closest_payload = payload;
                        }
                    }
                }
            }, object);

        }

        return closest_payload;
    }

    std::optional<HitPayload> any_hit(const Ray& ray, f32 t_min, f32 t_max) const {
        for (const auto& object: m_hittable_objects) {
            std::optional<HitPayload> hit_payload = std::visit(overloaded {
                [&](const auto& object) {
                    return object.hit(ray, t_min, t_max);
                }
            }, object);

            if (hit_payload.has_value()) {
                return hit_payload;
            }

        }
        return std::nullopt;
    }


private:
    std::vector<std::variant<Ts...>> m_hittable_objects;
};


class Sphere {
public:
    std::optional<HitPayload> hit(const Ray& ray, i32 t_min, i32 t_max) const;
    Vec3<f32> position() const { return m_position; }
    void set_position(const Vec3<f32>& pos) { m_position = pos; }
    Vec4<u8> object_color() const { return m_object_color; }
    Sphere(const Vec3<f32>& position, f32 radius, const Vec4<u8>& object_color): m_position(position), m_radius(radius), m_object_color(object_color) {}

private:
    Vec3<f32> m_position;    
    f32 m_radius = 0;
    Vec4<u8> m_object_color;
};


struct PointLight {
    PointLight(const Vec3<f32>& position, const Vec4<u8>& object_color): 
        position(position), object_color(object_color) {}
    Vec3<f32> position;    
    Vec4<u8> object_color;
};

using ObjectsList = HittableList<Sphere>;
using LightList = std::vector<std::variant<PointLight>>;


};
