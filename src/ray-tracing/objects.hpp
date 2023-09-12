#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/SameAsAny.hpp"
#include <algorithm>
#include <concepts>
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
    concept Hittable = requires (T& t, const Vec3<float>& position, const Ray& ray){
        { t.position() } -> std::same_as<Vec3<float>>;
        { t.set_position(position) } -> std::same_as<void>;
        { t.hit(ray) } -> std::same_as<HitPayload>;
    };
    

    template<Hittable...Ts>
    class HittableList {
    public:
        template<same_as_any<Ts...> T>
        auto add_object(T&& hittable_object) {
            return std::get<std::vector<T>>(m_hittable_objects).emplace_back(hittable_object);
        }

        auto hit(const Ray& ray) {
            std::vector<HitPayload> ret_vec;
            ret_vec.reserve((std::get<std::vector<Ts>>(m_hittable_objects).size() + ...));
            std::apply([&](const std::vector<Ts>&... v) {
                (std::for_each(v.begin(), v.end(), [&](auto object){ret_vec.emplace_back(object.hit(ray));}), ...);
            }, m_hittable_objects);
            return ret_vec;
        }
        
    private:
        std::tuple<std::vector<Ts>...> m_hittable_objects;
    };


    class Sphere {
    public:
        HitPayload hit(const Ray& ray);
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
    private:
        float radius = 0;
        Vec3<float> m_position;    
    };

    class Box {
    public:
        HitPayload hit(const Ray& ray) {return HitPayload{.t = 0.5};};
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
    private:
        Vec3<float> m_position;    
    };


    using ObjectsList = HittableList<Sphere, Box>;
};
