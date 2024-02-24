#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/Panic.hpp"
#include <cmath>
#include <fmt/core.h>
#include <optional>
#include <utils/ScopedTimer.hpp>

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

        HitPayload payload{ .material = this->material() };

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
        
        HitPayload payload{.material= this->material()};
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
        return payload;
    }

    std::optional<HitPayload> Triangle::hit(const Ray& ray, f32 t_min, f32 t_max) const {
        HitPayload payload{ .material = this->material() };
        f32 D = -m_normal.dot(m_vertices.x);
        payload.t = -(m_normal.dot(ray.origin) + D) / m_normal.dot(ray.direction);
        if (payload.t > t_max || payload.t < t_min) {
            return std::nullopt;
        }
        payload.hit_position = ray.origin + ray.direction * payload.t;


        Vec3<f32> C0 = payload.hit_position - m_vertices.x;
        if (m_normal.dot(m_edges.x.cross(C0)) <= 0) {
            return std::nullopt;
        }

        Vec3<f32> C1 = payload.hit_position - m_vertices.y;
        if (m_normal.dot(m_edges.y.cross(C1)) <= 0) {
            return std::nullopt; 
        } 

        Vec3<f32> C2 = payload.hit_position - m_vertices.z;
        if (m_normal.dot(m_edges.z.cross(C2)) <= 0) {
            return std::nullopt;
        }
        // P is inside the triangle

        payload.material = this->material();
        payload.normal = m_normal;

        return payload;
    }

    std::optional<HitPayload> Mesh::hit(const Ray& ray, f32 t_min, f32 t_max) const {
        std::optional<HitPayload> closest_payload = std::nullopt;
        for (const auto& tri : m_triangles) {
            auto payload = tri.hit(ray, t_min, t_max);
            if (payload.has_value()) {
                closest_payload = payload;
                t_min = payload->t;
            }
        }
        return closest_payload;
    }
};
