#include <chrono>
#include <cstdio>
#include <fmt/core.h>
#include <memory>
#include "linear_algebra/Vec3.decl.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "utils/Panic.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Scene.hpp"
#include "ray-tracing/objects.hpp"

using namespace RayTracer;

constexpr Vec3<f32> u8_color_to_float(Vec3<u8>&& color) {
    return Vec3<f32>(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
}

int main() {

    // CAMERA = Vec3(3.4589443, 2.62217, 12.626352) 0.20000002 -0.19999999
    // CAMERA = Vec3(0, 2.8092508, 14.059006) 0.20000002 0
    Camera cam(45, Vec3<f32>(3.4589443f, 2.62217f, 12.626352f), 0.2f, -0.2f, 1280, 720);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    // scene.add_object(Sphere(
    //     Vec3<f32>(10.f,  10.f, 10.f),
    //     5.0f,
    //     Material{
    //         .albedo = Vec3<f32>(1.0f),
    //         .roughness = 0.3f,
    //         .metalic = 0.0f,
    //         .emission_power = 120.0f,
    //     })
    // );

    scene.add_object(Sphere(
        Vec3<f32>(-3.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
            .roughness = 0.0f,
            .metalic = 0.f,
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(-1.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
            .roughness = 0.5f,
            .metalic = 0.f,
        })
    );
    scene.add_object(Sphere(
        Vec3<f32>(1.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.0f,
            .metalic = 1.f,
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(3.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.5f,
            .metalic = 1.f,
        })
    );

    scene.add_object(Box(
        Vec3<f32>(0.0f, 3.0f, 0.0f),
        10, 0.10f, 10,
        0,0,0,
        Material{
            .albedo = Vec3<f32>(1.0f),
            .roughness = 1.0f,
            .metalic = 1.0f,
            .emission_power = 40.0f,
        }
    ));

    scene.add_object(Box(
        Vec3<f32>(0.0f, -6.0f, 0.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.0f,
            .metalic = 1.0f,
        }
    ));
    
    scene.add_object(Box(
        Vec3<f32>(0.0f, 8.0f, 0.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.0f,
            .metalic = 0.0f,
        }
    ));




    scene.add_object(Box(
        Vec3<f32>(-10.0f, 0.0f, 0.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 200, 200)),
            .roughness = 0.f,
            .metalic = 1.0f,
        }
    ));
    
    scene.add_object(Box(
        Vec3<f32>(10.0f, 0.0f, 0.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 200, 200)),
            .roughness = 0.0f,
            .metalic = 1.0f,
        }
    ));

    scene.add_object(Box(
        Vec3<f32>(0.0f, 0.0f, -10.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.0f,
            .metalic = 1.0f,
        }
    ));
    
    scene.add_object(Box(
        Vec3<f32>(0.0f, 0.0f, 10.0f),
        10,10,10,
        0,0,0,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 255, 255)),
            .roughness = 0.0f,
            .metalic = 1.0f,
        }
    ));


    w.float_key_controls.push_back(FloatKeyControl{
        .increment_glfw_key = GLFW_KEY_1,
        .decrement_glfw_key = GLFW_KEY_2,
        .variable_to_change = &scene.get_object<Sphere>(0).m_material.roughness,
        .change_step = 0.1f,
    });

    w.float_key_controls.push_back(FloatKeyControl{
        .increment_glfw_key = GLFW_KEY_3,
        .decrement_glfw_key = GLFW_KEY_4,
        .variable_to_change = &scene.get_object<Sphere>(1).m_material.roughness,
        .change_step = 0.1f,
    });

   
    w.float_key_controls.push_back(FloatKeyControl{
        .increment_glfw_key = GLFW_KEY_5,
        .decrement_glfw_key = GLFW_KEY_6,
        .variable_to_change = &scene.get_object<Sphere>(2).m_material.roughness,
        .change_step = 0.1f,
    });

    w.float_key_controls.push_back(FloatKeyControl{
        .increment_glfw_key = GLFW_KEY_7,
        .decrement_glfw_key = GLFW_KEY_8,
        .variable_to_change = &scene.get_object<Sphere>(3).m_material.roughness,
        .change_step = 0.1f,
    });

    while(!glfwWindowShouldClose(w.m_glfw_window)) {
        glfwPollEvents();
        scene.render(10);    

        if (w.framebuffer_resized) {
            r.recreate_swap_chain();
            w.framebuffer_resized = false;
            r.wait_for_device_idle();
            usleep(100000);
            continue;
        }

        
        r.update_image(reinterpret_cast<u8*>(cam.image.data()));
        r.draw_frame();
    }
    
    r.wait_for_device_idle();
}
