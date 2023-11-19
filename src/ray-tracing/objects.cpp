#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/Panic.hpp"
#include <cmath>
#include <fmt/core.h>
#include <optional>


static f32 max_ignore_nan(f32 a, f32 b) {
    if (std::isnan(a)) {
        return b;
    }
    if (std::isnan(b)) {
        return a;
    }
    if (a < b) {
        return b;
    } 
    return a;
}

static f32 min_ignore_nan(f32 a, f32 b) {
    if (std::isnan(a)) {
        return b;
    }
    if (std::isnan(b)) {
        return a;
    }
    if (a > b) {
        return b;
    } 
    return a;
}

namespace RayTracer {
    std::optional<HitPayload> Sphere::hit(const Ray& ray, f32 t_min, f32 t_max) const {
        Vec3<f32> origin = ray.origin - this->position();
        f32 a = ray.direction.dot(ray.direction);
        f32 b = 2 * origin.dot(ray.direction);
        f32 c = origin.dot(origin) - this->m_radius*this->m_radius;
        f32 discriminant =  b * b - 4.0f * a * c;

        if (discriminant < 0.0) {
            return std::nullopt;
        }

        f32 root = (-b - std::sqrt(discriminant)) / (2.0f * a);
        if (root >= t_max || root <= t_min) {
            return std::nullopt;
        }

        // Vec3<f32> hit_point = origin + Vec3(ray.direction).scale(root);
        // Vec3<f32> normal = (hit_point - this->position()).scale(1.0f / this->m_radius);

        HitPayload payload;

        payload.hit_position = ray.origin + Vec3(ray.direction).scale(root);
        payload.normal = (payload.hit_position - this->position()).scale(1.0f / this->m_radius);
        if (ray.direction.dot(payload.normal) > 0) {
            payload.normal = -payload.normal;
        }
        payload.t = root;
        payload.material = this->material();
        
        return payload;
    }


    bool rayBoxIntersect(const Ray& ray, const Box& box, float& tNear, float& tFar) {
        tNear = -std::numeric_limits<float>::infinity();
        tFar = std::numeric_limits<float>::infinity();

        for (int i = 0; i < 3; ++i) {
            if (ray.direction[i] == 0.0f) {
                // Ray is parallel to the slab. No intersection if outside the slab.
                if (ray.origin[i] < box.m_box_min[i] || ray.origin[i] > box.m_box_max[i]) {
                    return false;
                }
            } else {
                float t1 = (box.m_box_min[i] - ray.origin[i]) / ray.direction[i];
                float t2 = (box.m_box_max[i] - ray.origin[i]) / ray.direction[i];

                if (t1 > t2) {
                    std::swap(t1, t2);
                }

                tNear = std::max(tNear, t1);
                tFar = std::min(tFar, t2);

                if (tNear > tFar) {
                    return false;
                }
            }
        }

        return true;
    }
    
    std::optional<HitPayload> Box::hit(const Ray& ray, f32 t_min, f32 t_max) const {
        
        float tNear, tFar;
        if (!rayBoxIntersect(ray, *this, tNear, tFar)) {
            return std::nullopt;
        }
        if (tNear < t_min || tNear > t_max) {
            return std::nullopt;
        }
        
        HitPayload payload;
        payload.hit_position = ray.origin + tNear * ray.direction;

        for (int i = 0; i < 3; ++i) {
            if (payload.hit_position[i] <= m_box_min[i] + 0.0001f) {
                payload.normal[i] = -1.0f;
            } else if (payload.hit_position[i] >= m_box_max[i] - 0.0001f) {
                payload.normal[i] = 1.0f;
            }
        }
        if (ray.direction.dot(payload.normal) > 0) {
            payload.normal = -payload.normal;
        }
        // payload.normal.normalize();
        payload.t = tNear;
        payload.material = this->material();
        
        return payload;
    }
};
