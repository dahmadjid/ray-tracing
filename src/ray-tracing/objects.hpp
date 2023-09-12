#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "utils/SameAsAny.hpp"
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
    concept Hittable = requires (T& t, const Vec3<float>& position){
        { t.position() } -> std::same_as<Vec3<float>>;
        { t.set_position(position) } -> std::same_as<void>;
        { t.hit() } -> std::same_as<HitPayload>;
    };
    

    template<Hittable...Ts>
    class HittableList {
    public:
        template<same_as_any<Ts...> T>
        auto add_object(T&& hittable_object) {
            return std::get<std::vector<T>>(m_hittable_objects).emplace_back(hittable_object);
        }
        
    private:
        std::tuple<std::vector<Ts>...> m_hittable_objects;
    };


    class Sphere {
    public:
        HitPayload hit();
        Vec3<float> position() const { return m_position; }
        void set_position(const Vec3<float>& pos) { m_position = pos; }
    private:
        float radius = 0;
        Vec3<float> m_position;    
    };


    using ObjectsList = HittableList<Sphere>;
};
