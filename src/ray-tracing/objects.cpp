#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cmath>
#include <fmt/core.h>


namespace RayTracer {
    std::optional<HitPayload> Sphere::hit(const Ray& ray, int t_min, int t_max) const {
        auto origin = ray.origin - this->position();
        auto a = ray.direction.dot(ray.direction);
        auto b = 2 * origin.dot(ray.direction);
        auto c = origin.dot(origin) - this->m_radius*this->m_radius;
        auto discriminant =  b * b - 4 * a * c;
        if (discriminant < 0.0) {
            return {};
        } else {
            auto discriminant_sqrted = std::sqrt(discriminant);
            float root = (-b - discriminant_sqrted) / (2.0 * a);
            if (root > t_max || root < t_min) {
                root = (-b + discriminant_sqrted ) / (2.0 * a);
                if (root > t_max || root < t_min) {
                    return {};
                } 
            }

            auto hit_point = origin + Vec3(ray.direction).scale(root);
            auto normal = (hit_point - this->m_position).normalize();

            if (ray.direction.dot(normal) > 0.0) {
                return HitPayload{
                    .hit_position=hit_point, 
                    .normal=-normal, 
                    .t=root, 
                    .front_face=false,  
                    .object_color=this->object_color()
                };

            } else {
                return HitPayload{
                    .hit_position=hit_point, 
                    .normal=normal , 
                    .t=root, 
                    .front_face=true ,  
                    .object_color=this->object_color()
                };
                
            }
        }
    }
};
