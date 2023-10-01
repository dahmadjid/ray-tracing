#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cmath>
#include <fmt/core.h>


namespace RayTracer {
    std::optional<HitPayload> Sphere::hit(const Ray& ray, i32 t_min, i32 t_max) const {
        Vec3<f32> origin = ray.origin - this->position();
        f32 a = ray.direction.dot(ray.direction);
        f32 b = 2 * origin.dot(ray.direction);
        f32 c = origin.dot(origin) - this->m_radius*this->m_radius;
        f32 discriminant =  b * b - 4 * a * c;
        if (discriminant < 0.0) {
            return {};
        } else {
            f32 discriminant_sqrted = std::sqrt(discriminant);
            f32 root = (-b - discriminant_sqrted) / (2.0 * a);
            if (root > t_max || root < t_min) {
                root = (-b + discriminant_sqrted ) / (2.0 * a);
                if (root > t_max || root < t_min) {
                    return {};
                } 
            }
            Vec3<f32> hit_point = origin + Vec3(ray.direction).scale(root);
            Vec3<f32> normal = (hit_point - this->position()).scale(1.0f / this->m_radius).normalize();
            
            return HitPayload{
                .hit_position=hit_point, 
                .normal=normal, 
                .t=root, 
                .front_face=true ,  
                .object_color=this->object_color()
            };
                
        }
    }

    std::optional<HitPayload> PointLight::hit(const Ray& ray, i32 t_min, i32 t_max) const {

        // check if this->m_position - ray.origin is a scalar multiple of ray.direction
        // x,y,z of res needs to be equal. any of them is t.
        Vec3 res = (this->m_position - ray.origin) / ray.direction;

        if (res.x == res.y && res.x == res.z) {
            if (res.x <= t_min || res.x >= t_max) {
                return {};
            }
            return HitPayload{
                .hit_position=this->m_position, 
                .normal=Vec3(0.f, 0.f, 0.f), 
                .t=res.x, 
                .front_face=true,  
                .object_color=this->object_color()
            };
        }

        return {};
    }
};
