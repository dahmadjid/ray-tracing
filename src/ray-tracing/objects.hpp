#pragma once

#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/SameAsAny.hpp"
#include <algorithm>
#include <concepts>
#include <optional>
#include <vector>

namespace RayTracer {
    struct HitPayload {
        Vec3<float> hit_position;
        Vec3<float> normal;
        float t = 0;
        bool front_face = false;
        Vec3<float> object_color;
    };

    template<typename T>
    concept Hittable = requires (T& t, const Vec3<float>& position, const Ray& ray, float t_min, float t_max){
        { t.position() } -> std::same_as<Vec3<float>>;
        { t.set_position(position) } -> std::same_as<void>;
        { t.hit(ray, t_min, t_max) } -> std::same_as<std::optional<HitPayload>>;
        { t.object_color() } -> std::same_as<Vec3<float>>;
    };
    

    template<Hittable...Ts>
    class HittableList {
    public:
        template<same_as_any<Ts...> T>
        void add_object(T&& hittable_object) {
            std::get<std::vector<T>>(m_hittable_objects).emplace_back(hittable_object);
        }

        std::vector<std::optional<HitPayload>> hit(const Ray& ray, float t_min, float t_max) const {
            std::vector<std::optional<HitPayload>> ret_vec;
            ret_vec.reserve((std::get<std::vector<Ts>>(m_hittable_objects).size() + ...));
            std::apply([&](const std::vector<Ts>&... v) {
                (std::for_each(v.begin(), v.end(), [&](auto object){ret_vec.emplace_back(object.hit(ray, t_min, t_max));}), ...);
            }, m_hittable_objects);
            return ret_vec;
        }
        
    private:
        std::tuple<std::vector<Ts>...> m_hittable_objects;
    };


    class Sphere {
    public:
        std::optional<HitPayload> hit(const Ray& ray, int t_min, int t_max);
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
        Vec3<float> object_color() { return m_object_color; }
        Sphere(const Vec3<float>& position, float radius, const Vec3<float>& object_color): m_position(position), m_radius(radius), m_object_color(object_color) {}
    private:
        Vec3<float> m_position;    
        float m_radius = 0;
        Vec3<float> m_object_color;
    };


    using ObjectsList = HittableList<Sphere>;
};
