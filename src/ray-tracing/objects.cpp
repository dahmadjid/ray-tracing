#include "ray-tracing/objects.hpp"

#include <fmt/core.h>

#include <cmath>
#include <limits>
#include <optional>
#include <utils/ScopedTimer.hpp>

#include "ray-tracing/Ray.hpp"
#include "utils/MathUtils.hpp"
#include <rfl/json.hpp>


struct Triangle_t {
    Vec3f x;
    Vec3f y;
    Vec3f z;
    Vec3f edge_x;
    Vec3f edge_y;
    Vec3f edge_z;
    Vec3f normal;
    f32 distance_to_origin;
    u16 material;
};

struct Ray_t {
    Vec3f origin;
    Vec3f direction;
};

struct DebugVectors_t {
    Vec3f x;
    Vec3f y;
    Vec3f z;
};

struct IntersectorInput {
    Triangle_t triangle;
    Ray_t ray;
};

struct HitInfo_t {
    Vec3f normal;
    u16 material;
    Vec3f hit_pos;
    f32 t;
};

struct IntersectorOutput {
    u8 hit;
    HitInfo_t hit_info;
    DebugVectors_t debug_vectors;
};

struct TestData {
    IntersectorInput input;
    IntersectorOutput output;
};

static std::vector<TestData> test_data = {};
static std::mutex mutex_test_data;

static void add_test_sample(RayTracer::Triangle tri, RayTracer::Ray ray, u8 hit, f32 t, Vec3f hit_pos, Vec3f normal_out, DebugVectors_t debug_vectors) {
    std::lock_guard l(mutex_test_data);
    if (test_data.size() > 49 && hit == false) {
        return;
    }
    auto input = IntersectorInput{
        .triangle =
            Triangle_t{
                .x = tri.m_vertices.x,
                .y = tri.m_vertices.y,
                .z = tri.m_vertices.z,
                .edge_x = tri.m_edges.x,
                .edge_y = tri.m_edges.y,
                .edge_z = tri.m_edges.z,
                .normal = tri.m_normal,
                .distance_to_origin = -tri.m_normal.dot(tri.m_vertices.x),
                .material = 2,
            },
        .ray = {.origin = ray.origin, .direction=ray.direction}
    };

    auto output = IntersectorOutput{
        .hit = hit, 
        .hit_info = HitInfo_t{
            .normal = normal_out,
            .material = 2,
            .hit_pos = hit_pos,
            .t = t
        },
        .debug_vectors = debug_vectors
    };
    
    test_data.push_back(TestData{input, output});
    if (test_data.size() == 100) {
        auto test = rfl::json::write(test_data);
        rfl::json::save("tri_intersector_test_data.json", test_data);
        exit(0);
    }
}


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

std::optional<HitPayload> Triangle::hit(const Ray& ray, f32 t_min, f32 t_max) const {
    u32 seed = 1231;
    f32 NdotRayDir = m_normal.dot(ray.direction);
    // this means NdotV is negative, this means we are looking at a triangle from behind
    if (NdotRayDir > 0.0f) {
        if (rand_float(seed) < 0.1f) {
            add_test_sample(*this, ray, 0, 0, Vec3f(), m_normal, {});
        }
        return std::nullopt;
    }

    HitPayload payload{.material = this->material()};
    f32 D = -m_normal.dot(m_vertices.x);
    payload.t = -(m_normal.dot(ray.origin) + D) / NdotRayDir;
    if (payload.t > t_max || payload.t < t_min) {
        if (rand_float(seed) < 0.1f) {
            add_test_sample(*this, ray, 0, payload.t, Vec3f(), Vec3f(), {});
        }
        return std::nullopt;
    }
    payload.hit_position = ray.origin + ray.direction * payload.t;

    Vec3<f32> C0 = payload.hit_position - m_vertices.x;
    if (m_normal.dot(m_edges.x.cross(C0)) <= 0) {
        if (rand_float(seed) < 0.1f) {
            add_test_sample(*this, ray, 0, payload.t, payload.hit_position, m_normal, {Vec3(payload.t), Vec3f(), Vec3f()});
        }
        return std::nullopt;
    }

    Vec3<f32> C1 = payload.hit_position - m_vertices.y;
    if (m_normal.dot(m_edges.y.cross(C1)) <= 0) {
        if (rand_float(seed) < 0.1f) {
            add_test_sample(*this, ray, 0, payload.t, payload.hit_position, m_normal, {Vec3(payload.t), C1, Vec3f()});
        }
        return std::nullopt;
    }

    Vec3<f32> C2 = payload.hit_position - m_vertices.z;
    if (m_normal.dot(m_edges.z.cross(C2)) <= 0) {
        if (rand_float(seed) < 0.1f) {
            add_test_sample(*this, ray, 0, payload.t, payload.hit_position, m_normal, {Vec3(payload.t), C1, C2});
        }
        return std::nullopt;
    }
    // P is inside the triangle

    payload.material = this->material();
    payload.normal = m_normal;

    if (rand_float(seed) < 0.5f) {
        add_test_sample(*this, ray, 1, payload.t, payload.hit_position, m_normal, {Vec3(payload.t), C1, C2});
    }

    return payload;
}

// https://www.realtimerendering.com/raytracinggems/unofficial_RayTracingGems_v1.9.pdf
// 16.5.2
std::pair<Vec3f, f32> Triangle::sample(u32& seed) const {
    f32 u0 = rand_float(seed);
    f32 u1 = rand_float(seed);
    f32 beta = 1 - std::sqrt(u0);
    f32 gamma = (1 - beta) * u1;
    f32 alpha = 1 - beta - gamma;
    return {
        Vec3f(alpha * this->m_vertices[0] + beta * this->m_vertices[1] + gamma * this->m_vertices[2]), m_sampling_pdf
    };
}

f32 Triangle::pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) const {
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

std::pair<Vec3f, f32> Mesh::sample(u32& seed) const {
    f32 random = rand_float(seed);
    u32 random_selected_index = (u32)std::floor(random * this->m_triangles.size());
    return this->m_triangles[random_selected_index].sample(seed);
}

f32 Mesh::pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) const {
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

    return tri.m_sampling_pdf / (f32)m_triangles.size();
}
};  // namespace RayTracer
