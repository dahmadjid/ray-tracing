#pragma once

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Quaternion.hpp"
#include "utils/MathUtils.hpp"
#include <array>
#include <cstdint>
#include <ranges>
#include <sys/types.h>
#include <vector>
#include <memory>

namespace RayTracer {


template<u32 window_width, u32 window_height>
class Camera {
    f32 m_viewport_height = 0;
    f32 m_viewport_width = 0;
    Vec3<f32> m_z_axis;
    Vec3<f32> m_position;


public:
    std::unique_ptr<std::array<Vec4<u8>, window_width * window_height>> image;
    std::unique_ptr<std::array<Vec3<f32>, window_height * window_width>> ray_directions;
    std::unique_ptr<std::array<Vec4<u32>, window_width * window_height>> accumulation_data;
    u32 frame_index = 1;
    
    Camera(f32 vfov) :  
        m_position(Vec3(0.f, 0.f, 30.f)),
        ray_directions(std::make_unique<std::array<Vec3<f32>, window_height * window_width>>()),
        image(std::make_unique<std::array<Vec4<u8>, window_height * window_width>>()),
        accumulation_data(std::make_unique<std::array<Vec4<u32>, window_width * window_height>>()) 

    {

        auto theta = to_radians(vfov);
        auto h = std::tan(theta/2.0);
        auto viewport_height =  1.0 * h;
        auto viewport_width =  viewport_height * window_width / window_height;

        auto z_axis = Vec3(0.0f, 0.0f, -1.0f);


        m_viewport_height = viewport_height;
        m_viewport_width = viewport_width;
        m_z_axis = z_axis;

        this->rotate(0, 0);
        this->calculate_ray_directions();
    
    }

    void calculate_ray_directions() {
        auto up = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up).normalize().scale(m_viewport_width/2.0);
        auto up_direction = m_z_axis.cross(right_direction).normalize().scale(m_viewport_height/2.0);
        for (int y = window_height - 1; y >= 0; y--) {
            auto v = static_cast<f32>(y) / static_cast<f32>(window_height) * 2.0 - 1.0;
            for (u32 x = 0; x < window_width; x++) {
                auto u = static_cast<f32>(x) / static_cast<f32>(window_width) * 2.0 - 1.0;
                this->ray_directions->operator[](x + y * window_width) = (m_z_axis + Vec3(right_direction).scale(u) + Vec3(up_direction).scale(v));
            }
        }
    }


    void update_x_position(f32 x) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        m_position = m_position + m_z_axis.cross(up_dir).scale(x);
        accumulation_data->fill(Vec4<u32>());
        this->frame_index = 1;

    }

    void update_y_position(f32 y) {
        m_position.y += y;
        accumulation_data->fill(Vec4<u32>());
        this->frame_index = 1;
    }
    
    void update_z_position(f32 z) {
        m_position = m_position + Vec3(m_z_axis).scale(z);
        accumulation_data->fill(Vec4<u32>());
        this->frame_index = 1;
    }

    void rotate(f32 pitch_delta_radians, f32 yaw_delta_radians) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up_dir).normalize();
        auto up = m_z_axis.cross(right_direction).normalize();
        m_z_axis.rotate(Quaternion<f32>::angle_axis(-pitch_delta_radians, right_direction).cross(Quaternion<f32>::angle_axis(yaw_delta_radians, up)).normalize());
        accumulation_data->fill(Vec4<u32>());
        this->frame_index = 1;
    }


    Vec3<f32> position() const {
        return m_position;
    }

    Vec3<f32> get_ray(u32 x, u32 y) {
        return this->ray_directions->operator[](x + y * window_width);
    }
};
}