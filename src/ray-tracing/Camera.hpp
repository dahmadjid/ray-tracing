#pragma once

#include "linear_algebra/Vec3.hpp"
#include "linear_algebra/Quaternion.hpp"
#include "utils/MathUtils.hpp"
#include <cstdint>
#include <ranges>
#include <sys/types.h>
#include <vector>
#include <memory>

namespace RayTracer {


class Camera {
    f32 m_viewport_height = 0;
    f32 m_viewport_width = 0;
    Vec3<f32> m_z_axis;
    Vec3<f32> m_position;
    f32 m_vfov = 0;

public:
    std::vector<Vec4<u8>> image;
    std::vector<Vec3<f32>> ray_directions;
    std::vector<Vec3<f32>> accumulation_data;
    u32 frame_index = 1;
    u32 window_width;
    u32 window_height;
    
    Camera(f32 vfov, Vec3<f32> position, f32 pitch, f32 yaw, u32 w_width, u32 w_height) :  
        m_position(position), m_vfov(vfov)
    {
        resize_camera(w_width, w_height);

        m_z_axis = Vec3(0.0f, 0.0f, -1.0f);;

        this->rotate(pitch, yaw);
        this->calculate_ray_directions();
    
    }

    void resize_camera(u32 w_width, u32 w_height) {
        this->window_width = w_width;
        this->window_height = w_height;
        this->image.resize(window_height * window_width);
        this->accumulation_data.resize(window_height * window_width);
        this->ray_directions.resize(window_height * window_width);

        f32 theta = to_radians(m_vfov);
        f32 h = std::tan(theta / 2.0f);
        f32 viewport_height = 1.0f * h;
        f32 viewport_width = viewport_height * (f32)window_width / (f32)window_height;
        m_viewport_height = viewport_height;
        m_viewport_width = viewport_width;
        memset(this->accumulation_data.data(), 0, this->accumulation_data.size() * sizeof(Vec3<f32>));
        this->frame_index = 1;
    }

    Camera(Camera&) = delete;
    Camera& operator=(Camera&) = delete;

    void calculate_ray_directions() {
        auto up = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up).normalize().scale(m_viewport_width / 2.0f);
        auto up_direction = m_z_axis.cross(right_direction).normalize().scale(m_viewport_height / 2.0f);
        for (i32 y = (i32)window_height - 1; y >= 0; y--) {
            f32 v = static_cast<f32>(y) / static_cast<f32>(window_height) * 2.0f - 1.0f;
            for (u32 x = 0; x < window_width; x++) {
                f32 u = static_cast<f32>(x) / static_cast<f32>(window_width) * 2.0f - 1.0f;
                this->ray_directions[x + y * window_width] = (m_z_axis + Vec3(right_direction).scale(u) + Vec3(up_direction).scale(v));
            }
        }
    }



    void update_x_position(f32 x) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        m_position = m_position + m_z_axis.cross(up_dir).scale(x);
        reset_accu_data();

    }

    void update_y_position(f32 y) {
        m_position.y += y;
        reset_accu_data();
    }
    
    void update_z_position(f32 z) {
        m_position = m_position + Vec3(m_z_axis).scale(z);
        reset_accu_data();
    }

    void rotate(f32 pitch_delta_radians, f32 yaw_delta_radians) {
        auto up_dir = Vec3(0.0f, 1.0f, 0.0f);
        auto right_direction = m_z_axis.cross(up_dir).normalize();
        auto up = m_z_axis.cross(right_direction).normalize();
        m_z_axis.rotate(Quaternion<f32>::angle_axis(-pitch_delta_radians, right_direction).cross(Quaternion<f32>::angle_axis(yaw_delta_radians, up)).normalize());
        reset_accu_data();
    }

    void reset_accu_data() {
        memset(this->accumulation_data.data(), 0, this->accumulation_data.size() * sizeof(Vec3<f32>));
        this->frame_index = 1;
    }


    Vec3<f32> position() const {
        return m_position;
    }

    Vec3<f32> get_ray(u32 x, u32 y) {
        return this->ray_directions[x + y * window_width];
    }

    
};
}