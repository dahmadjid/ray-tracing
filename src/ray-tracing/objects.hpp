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

// ::begin parameters
// color baseColor .82 .67 .16
// float metallic 0 1 0
// float subsurface 0 1 0
// float specular 0 1 .5
// float roughness 0 1 .5
// float specularTint 0 1 0
// float anisotropic 0 1 0
// float sheen 0 1 0
// float sheenTint 0 1 .5
// float clearcoat 0 1 0
// float clearcoatGloss 0 1 1
// ::end parameters

struct Material {
    Vec3<f32> albedo;
    f32 roughness = 0.5f;
    f32 metalic = 0.0f;
    f32 emission_power = 0.0f;
    f32 ior = 1.5f;
    f32 subsurface = 0.0f;
    f32 specular = 0.5f;
    f32 specular_tint = 0.0f;
    f32 anisotropic = 0.0f;
    f32 sheen = 0.0f;
    f32 sheen_tint = 0.5f;
    f32 clearcoat = 0.0f;
    f32 clearcoat_gloss = 1.0f;

    Vec3<f32> get_emission() {
        return Vec3(this->albedo).scale(emission_power);
    };

};

struct HitPayload {
    Vec3<f32> hit_position;
    Vec3<f32> normal;
    f32 t = 0;
    bool front_face = false;
    Material material;
};

template<typename T>
concept Hittable = requires (T& object, const Vec3<f32>& position, const Ray& ray, f32 t_min, f32 t_max){
    { object.position() } -> std::same_as<Vec3<f32>>;
    { object.set_position(position) } -> std::same_as<void>;
    { object.hit(ray, t_min, t_max) } -> std::same_as<std::optional<HitPayload>>;
    { object.material() } -> std::same_as<Material>;
};



template<Hittable...Ts>
class HittableList {
public:
    template<same_as_any<Ts...>  T>
    void add_object(T&& hittable_object) {
        m_hittable_objects.emplace_back(std::forward<T>(hittable_object));
    }
    
    template<same_as_any<Ts...>  T>
    T& get_object(u32 index) {
        return std::get<T>(m_hittable_objects[index]);
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
                [&](const auto& object) -> std::optional<HitPayload> {
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


struct Sphere {
    std::optional<HitPayload> hit(const Ray& ray, f32 t_min, f32 t_max) const;
    Vec3<f32> position() const { return m_position; }
    void set_position(const Vec3<f32>& pos) { m_position = pos; }
    Material material() const { return m_material; }
    Sphere(
        const Vec3<f32>& position, 
        f32 radius, 
        const Material& material
    ) : m_position(position), m_radius(radius), m_material(material) {}
    Vec3<f32> m_position;    
    f32 m_radius = 0;
    Material m_material;
};

struct Box {
    std::optional<HitPayload> hit(const Ray& ray, f32 t_min, f32 t_max) const;
    Vec3<f32> position() const { return m_position; }
    void set_position(const Vec3<f32>& pos) { m_position = pos; }
    Material material() const { return m_material; }
    Box(
        const Vec3<f32>& position, 
        f32 width, 
        f32 height, 
        f32 depth, 
        f32 pitch,
        f32 roll,
        f32 yaw,
        const Material& material
    ) : m_position(position), m_material(material), m_pitch(pitch), m_roll(roll), m_yaw(yaw), m_width(width), m_height(height), m_depth(depth), m_halves(width/2, height/2, depth/2) {
        this->m_box_max = m_position + m_halves;
        this->m_box_min = m_position - m_halves;
        
    }

   
    Vec3<f32> m_position;    
    Material m_material;
    Vec3<f32> m_box_max;
    Vec3<f32> m_box_min; 
    f32 m_pitch = 0.0f;
    f32 m_roll = 0.0f;
    f32 m_yaw = 0.0f;
    f32 m_width;
    f32 m_height;
    f32 m_depth;
    Vec3<f32> m_halves;
};

struct PointLight {
    PointLight(const Vec3<f32>& position, const Vec3<f32>& color): 
        position(position), color(color) {}
    Vec3<f32> position;    
    Vec3<f32> color;
};

using ObjectsList = HittableList<Sphere, Box>;
using LightList = std::vector<std::variant<PointLight>>;


};
