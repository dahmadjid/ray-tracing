#pragma once

#include <fmt/core.h>

#include <optional>
#include <variant>
#include <vector>

#include "Material.hpp"
#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/Obj.hpp"
#include "utils/Overloaded.hpp"
#include "utils/SameAsAny.hpp"

namespace RayTracer {
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

struct HitPayload {
    Vec3f hit_position;
    Vec3f normal;
    f32 t = 0;
    bool front_face = false;
    Material material;
};

template <typename T>
concept Hittable = requires(T& object, const Vec3f& position, const Ray& ray, f32 t_min, f32 t_max) {
    { object.position() } -> std::same_as<Vec3f>;
    { object.set_position(position) } -> std::same_as<void>;
    { object.hit(ray, t_min, t_max) } -> std::same_as<std::optional<HitPayload>>;
    { object.material() } -> std::same_as<Material>;
};

template <Hittable... Ts>
class HittableList {
public:
    template <same_as_any<Ts...> T>
    void add_object(T&& hittable_object) {
        m_hittable_objects.emplace_back(std::forward<T>(hittable_object));
    }

    template <same_as_any<Ts...> T>
    const T& get_object(u32 index) const {
        return std::get<T>(m_hittable_objects[index]);
    }

    template <same_as_any<Ts...> T>
    T& get_object(u32 index) {
        return std::get<T>(m_hittable_objects[index]);
    }

    std::optional<HitPayload> closest_hit(const Ray& ray, f32 t_min, f32 t_max) const {
        std::optional<HitPayload> closest_payload = std::nullopt;
        for (const auto& object : m_hittable_objects) {
            std::visit(
                overloaded{[&](const auto& object) {
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
                }},
                object
            );
        }

        return closest_payload;
    }

    std::optional<HitPayload> any_hit(const Ray& ray, f32 t_min, f32 t_max) const {
        for (const auto& object : m_hittable_objects) {
            std::optional<HitPayload> hit_payload = std::visit(
                overloaded{[&](const auto& object) -> std::optional<HitPayload> {
                    return object.hit(ray, t_min, t_max);
                }},
                object
            );

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

    Vec3f position() const {
        return m_position;
    }

    void set_position(const Vec3f& pos) {
        m_position = pos;
    }

    Material material() const {
        return m_material;
    }

    Sphere(const Vec3f& position, f32 radius, const Material& material)
        : m_position(position), m_radius(radius), m_material(material) {}

    Vec3f m_position;
    f32 m_radius = 0;
    Material m_material;
};

struct Triangle {
    std::optional<HitPayload> hit(const Ray& ray, f32 t_min, f32 t_max) const;

    Vec3f position() const {
        return m_position;
    }

    void set_position(const Vec3f& pos) {
        m_position = pos;
    }

    Material material() const {
        return m_material;
    }

    // position in object and pdf
    std::pair<Vec3f, f32> sample(u32& seed) const;
    f32 pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) const;

    Triangle(const Vec3f& position, const Material& material, const Vec3<Vec3f>& vertices)
        : m_position(position), m_material(material), m_vertices(vertices) {
        m_edges.x = m_vertices.y - m_vertices.x;
        m_edges.y = m_vertices.z - m_vertices.y;
        m_edges.z = m_vertices.x - m_vertices.z;
        m_normal = (m_edges.x.cross(m_edges.y)).normalize();

        // https://math.stackexchange.com/questions/128991/how-to-calculate-the-area-of-a-3d-triangle
        Vec3f ab = m_vertices.x - m_vertices.y;
        Vec3f ac = m_vertices.x - m_vertices.z;
        f32 ABdotAC = ab.dot(ac);
        f32 area = 0.5f * std::sqrt(ab.length_squared() * ac.length_squared() - ABdotAC * ABdotAC);
        m_sampling_pdf = 1.0f / area;
    }

    Vec3f m_position;
    Material m_material;
    Vec3<Vec3f> m_vertices;
    Vec3<Vec3f> m_edges;
    Vec3f m_normal;
    f32 m_sampling_pdf;
};

struct Mesh {
    std::optional<HitPayload> hit(const Ray& ray, f32 t_min, f32 t_max) const;

    Vec3f position() const {
        return m_position;
    }

    void set_position(const Vec3f& pos) {
        m_position = pos;
    }

    Material material() const {
        return m_material;
    }

    // position in object and pdf
    // pdf can NOT be 0 in this case
    std::pair<Vec3f, f32> sample(u32& seed) const;

    // pdf CAN be 0
    f32 pdf(const Vec3f& sampled_light_dir, const Vec3f& hit_position, const Vec3f& hit_normal) const;
    std::optional<u32> get_intersecting_triangle(const Ray& ray, f32 t_min, f32 t_max) const;

    Mesh(const Vec3f& position, const Material& material, const ParsedObj& obj)
        : m_position(position), m_material(material) {
        m_triangles.reserve(obj.faces.size());
        for (const Vec3<Vec3<i32>>& face_indices : obj.faces) {
            // -1 because .obj starts index at 1
            Vec3f v0 = obj.vertices[face_indices.x.x - 1] + position;
            Vec3f v1 = obj.vertices[face_indices.y.x - 1] + position;
            Vec3f v2 = obj.vertices[face_indices.z.x - 1] + position;

            Coordinate u0 = obj.uv_map[face_indices.x.y - 1];
            Coordinate u1 = obj.uv_map[face_indices.y.y - 1];
            Coordinate u2 = obj.uv_map[face_indices.z.y - 1];

            Vec3f n0 = obj.vertex_normals[face_indices.x.z - 1];
            Vec3f n1 = obj.vertex_normals[face_indices.y.z - 1];
            Vec3f n2 = obj.vertex_normals[face_indices.z.z - 1];
            Vec3<f32> average = (n0 + n1 + n2) / 3.0f;
            Triangle tri = Triangle(Vec3(0.0f), m_material, Vec3(v0, v1, v2));
            if (average.dot(tri.m_normal) < 0) {
                tri.m_normal = -tri.m_normal;
            }
            m_triangles.emplace_back(std::move(tri));
        }
    }

    Vec3f m_position;
    Material m_material;
    std::vector<Triangle> m_triangles;
};


struct AABB {
    Vec3f bmin = Vec3f(1e30f); 
    Vec3f bmax = Vec3f(-1e30f);

    void grow(const Vec3f& p) {
        bmin = Vec3f::min(bmin, p);
        bmax = Vec3f::max(bmax, p);
    }

    f32 area() const {
        Vec3f box_extent = bmax - bmin;
        return box_extent.x * box_extent.y + box_extent.y * box_extent.z + box_extent.z * box_extent.x;
    }
};

struct BVHNode {
    AABB aabb;
    u16 left_first;
    u16 triangle_count;
};

using ObjectsList = HittableList<Sphere, Triangle, Mesh>;

};  // namespace RayTracer
