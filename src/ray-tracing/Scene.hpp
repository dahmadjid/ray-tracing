#pragma once

#include "linear_algebra/Vec4.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/objects.hpp"
#include "ray-tracing/Ray.hpp"
#include <cstdint>
#include <utility>
#include <array>


namespace RayTracer {

template<uint32_t window_width, uint32_t window_height>
class Scene {
    ObjectsList m_objects;
    Camera<window_width, window_height>& m_camera;
    const uint32_t m_window_width;
    const uint32_t m_window_height;


public:
    
    Scene(Camera<window_width, window_height>& camera)
        : m_camera(camera), m_window_width(window_width), m_window_height(window_height) {} 
    
    Scene(Scene&) = delete;
    Scene& operator=(Scene&) = delete;

    template<typename T>
    void add_object(T&& hittable_object) {
        m_objects.add_object(std::forward(hittable_object));
    }

    std::array<Vec4<uint8_t>, window_height * window_width> render() {
        std::array<Vec4<uint8_t>, window_height * window_width> image;

        for (int y = m_window_height - 1; y >= 0; y--) {
            for (uint32_t x = 0; x < m_window_width; x++) {

                // auto ray_direction = this->m_camera.m_ray_directions[(x + y * window_width)];
                // auto ray = Ray{origin: this->m_camera.position.clone(), direction: ray_direction};
                // auto color = ray.color(&this->m_objects, 0.01, f64::INFINITY, 2);

                image[x + y * m_window_width] = Vec4<uint8_t>(0, 0, 0, 0);
            }
        }

        return image;
        
    }

};  

};
