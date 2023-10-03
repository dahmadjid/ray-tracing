#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cmath>
#include <fmt/core.h>
#include <optional>


namespace RayTracer {
    std::optional<HitPayload> Sphere::hit(const Ray& ray, i32 t_min, i32 t_max) const {
        Vec3<f32> origin = ray.origin - this->position();
        f32 a = ray.direction.dot(ray.direction);
        f32 b = 2 * origin.dot(ray.direction);
        f32 c = origin.dot(origin) - this->m_radius*this->m_radius;
        f32 discriminant =  b * b - 4.0f * a * c;

        if (discriminant < 0.0) {
            return std::nullopt;
        }

        f32 root = (-b - std::sqrt(discriminant)) / (2.0f * a);
        if (root > t_max || root < t_min) {
            return std::nullopt;
        }

        // Vec3<f32> hit_point = origin + Vec3(ray.direction).scale(root);
        // Vec3<f32> normal = (hit_point - this->position()).scale(1.0f / this->m_radius);

        HitPayload payload;

        payload.hit_position = origin + Vec3(ray.direction).scale(root);
        payload.normal = payload.hit_position.normalize();
        payload.hit_position += this->position();
        payload.t = root;
        payload.object_color = this->object_color();
        
        return payload;

        // return HitPayload{
        //     .hit_position=hit_point, 
        //     .normal=normal, 
        //     .t=root, 
        //     .front_face=true ,  
        //     .object_color=this->object_color()
        // };
                
    }
};
