#pragma once

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/SameAsAny.hpp"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <optional>
#include <vector>

namespace RayTracer {
    struct HitPayload {
        Vec3<float> hit_position;
        Vec3<float> normal;
        float t = 0;
        bool front_face = false;
        Vec4<uint8_t> object_color;
    };

    template<typename T>
    concept Hittable = requires (T& object, const Vec3<float>& position, const Ray& ray, float t_min, float t_max){
        { object.position() } -> std::same_as<Vec3<float>>;
        { object.set_position(position) } -> std::same_as<void>;
        { object.hit(ray, t_min, t_max) } -> std::same_as<std::optional<HitPayload>>;
        { object.object_color() } -> std::same_as<Vec4<uint8_t>>;
    };
    

    template<Hittable...Ts>
    class HittableList {
    public:
        template<typename T>
        void add_object(T&& hittable_object) {
            std::get<std::vector<T>>(m_hittable_objects).emplace_back(hittable_object);
        }

        std::optional<HitPayload> hit(const Ray& ray, float t_min, float t_max) const {
            std::vector<std::optional<HitPayload>> payloads;
            
            payloads.reserve((std::get<std::vector<Ts>>(m_hittable_objects).size() + ...));
            
            std::apply([&](const std::vector<Ts>&... v) {
                (std::for_each(v.begin(), v.end(), [&](const auto& object) {
                    payloads.push_back(object.hit(ray, t_min, t_max));
                }), ...);
            }, m_hittable_objects);

            std::optional<HitPayload> closest_payload;

            for (const auto& payload: payloads) {
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

            return closest_payload;
        }

    private:
        std::tuple<std::vector<Ts>...> m_hittable_objects;
    };


    class Sphere {
    public:
        std::optional<HitPayload> hit(const Ray& ray, int t_min, int t_max) const;
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
        Vec4<uint8_t> object_color() const { return m_object_color; }
        Sphere(const Vec3<float>& position, float radius, const Vec4<uint8_t>& object_color): m_position(position), m_radius(radius), m_object_color(object_color) {}
    private:
        Vec3<float> m_position;    
        float m_radius = 0;
        Vec4<uint8_t> m_object_color;
    };

    class Box {
    public:
        // std::optional<HitPayload> hit(const Ray& ray, int t_min, int t_max) const;
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
        Vec4<uint8_t> object_color() const { return m_object_color; }
        Box(const Vec3<float>& position, float radius, const Vec4<uint8_t>& object_color): m_position(position), m_radius(radius), m_object_color(object_color) {}
    private:
        Vec3<float> m_position;    
        float m_radius = 0;
        Vec4<uint8_t> m_object_color;
    };


    using ObjectsList = HittableList<Sphere>;
};
