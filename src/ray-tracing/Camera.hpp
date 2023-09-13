#pragma once

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Quaternion.hpp"
#include "utils/MathUtils.hpp"
#include <array>
#include <cstdint>
#include <ranges>
#include <sys/types.h>
#include <vector>


namespace RayTracer {


template<uint32_t window_width, uint32_t window_height>
class Camera {
    float m_viewport_height = 0;
    float m_viewport_width = 0;
    const uint32_t m_window_width = window_width;
    const uint32_t m_window_height = window_height;
    Vec3<float> m_z_axis;
    Vec3<float> m_position;
    std::array<Vec3<float>, window_width * window_height> m_ray_directions;



public:
    Camera(float vfov)
    : m_position(Vec3(0.f, 0.f, 6.f)) {
        auto theta = to_radians(vfov);
        auto h = std::tan(theta/2.0);
        auto viewport_height =  1.0 * h;
        auto viewport_width =  viewport_height * window_width / window_height;

        auto z_axis = Vec3(0.0f, 0.0f, -1.0f);
        std::vector<Vec3<float>> ray_directions;
        ray_directions.reserve(window_width * window_height);        
        m_viewport_height = viewport_height;
        m_viewport_width = viewport_width;
        m_z_axis = z_axis;

        this->calculate_ray_directions();
    }

    void calculate_ray_directions() {
        auto up = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up).normalize().scale(m_viewport_width/2.0);
        auto up_direction = m_z_axis.cross(right_direction).normalize().scale(m_viewport_height/2.0);
        
        for (int y = m_window_height - 1; y >= 0; y--) {
            auto v = static_cast<float>(y) / static_cast<float>(m_window_height) * 2.0 - 1.0;
            for (uint32_t x = 0; x < m_window_width; x++) {
                auto u = static_cast<float>(x) / static_cast<float>(m_window_width) * 2.0 - 1.0;
                m_ray_directions[x + y * window_width] = (m_z_axis + Vec3(right_direction).scale(u) + Vec3(up_direction).scale(v));
            }
        }
    }


    auto ray_directions() const {
        return m_ray_directions;
    }
    void update_x_position(float x) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        m_position = m_position + m_z_axis.cross(up_dir).scale(x);
    }

    void update_y_position(float y) {
        m_position.y += y;
    }
    
    void update_z_position(float z) {
        m_position = m_position + Vec3(m_z_axis).scale(z);
    }

    void rotate(float pitch_delta_radians, float yaw_delta_radians) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up_dir).normalize();
        m_z_axis.rotate(Quaternion<float>::angle_axis(-pitch_delta_radians, right_direction).cross(Quaternion<float>::angle_axis(-yaw_delta_radians, up_dir)).normalize());
    }


    Vec3<float> position() const {
        return m_position;
    }
};
}