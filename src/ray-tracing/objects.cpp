#include "ray-tracing/objects.hpp"

#include <fmt/core.h>

#include <cmath>
#include <limits>
#include <optional>
#include <utils/ScopedTimer.hpp>

#include "ray-tracing/Ray.hpp"
#include "utils/MathUtils.hpp"

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
    f32 c = origin.dot(origin) - this->m_radius * this->m_radius;
    f32 discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0) {
        return std::nullopt;
    }

    f32 root = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (root >= t_max || root <= t_min) {
        return std::nullopt;
    }

    HitPayload payload{.material = this->material()};

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

    HitPayload payload{.material = this->material()};
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
    f32 NdotRayDir = m_normal.dot(ray.direction);
    // this means NdotV is negative, this means we are looking at a triangle from behind
    if (NdotRayDir > 0.0f) {
        return std::nullopt;
    }

    HitPayload payload{.material = this->material()};
    f32 D = -m_normal.dot(m_vertices.x);
    payload.t = -(m_normal.dot(ray.origin) + D) / NdotRayDir;
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

// https://www.realtimerendering.com/raytracinggems/unofficial_RayTracingGems_v1.9.pdf
// 16.5.2
std::pair<Vec3f, f32> Triangle::sample(u32& seed) {
    f32 u0 = rand_float(seed);
    f32 u1 = rand_float(seed);
    f32 beta = 1 - std::sqrt(u0);
    f32 gamma = (1 - beta) * u1;
    f32 alpha = 1 - beta - gamma;
    return {
        Vec3f(alpha * this->m_vertices[0] + beta * this->m_vertices[1] + gamma * this->m_vertices[2]), m_sampling_pdf
    };
}

f32 Triangle::pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) {
    if (!this->hit(Ray{.origin = hit_position, .direction = sampled_light_dir}, 0.001f, std::numeric_limits<f32>::max())
             .has_value()) {
        return 0.0f;
    }

    if (this->m_normal.dot(sampled_light_dir) >= 0) {
        return 0.0f;
    }

    return m_sampling_pdf;
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

std::optional<u32> Mesh::get_intersecting_triangle(const Ray& ray, f32 t_min, f32 t_max) const {
    std::optional<u32> closest_triangle = std::nullopt;

    for (u32 i = 0; i < m_triangles.size(); ++i) {
        auto payload = m_triangles[i].hit(ray, t_min, t_max);
        if (payload.has_value()) {
            closest_triangle = i;
            t_min = payload->t;
        }
    }
    return closest_triangle;
}

std::pair<Vec3f, f32> Mesh::sample(u32& seed) {
    f32 random = rand_float(seed);
    u32 random_selected_index = (u32)std::floor(random * this->m_triangles.size());
    return this->m_triangles[random_selected_index].sample(seed);
}

f32 Mesh::pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) {
    auto triangle_index = this->get_intersecting_triangle(
        Ray{.origin = hit_position, .direction = sampled_light_dir}, 0.001f, std::numeric_limits<f32>::max()
    );
    if (!triangle_index.has_value()) {
        return 0.0f;
    }

    Triangle tri = m_triangles[*triangle_index];
    if (tri.m_normal.dot(sampled_light_dir) >= 0) {
        return 0.0f;
    }

    return tri.m_sampling_pdf;
}
};  // namespace RayTracer
