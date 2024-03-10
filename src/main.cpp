#include <chrono>
#include <cstdio>
#include <fmt/core.h>
#include <memory>
#include <random>
#include <thread>
#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/Material.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/MathUtils.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "utils/Panic.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Scene.hpp"
#include "ray-tracing/objects.hpp"
#include "utils/Obj.hpp"
#include "utils/ScopedTimer.hpp"

using namespace RayTracer;

constexpr Vec3f u8_color_to_float(Vec3<u8>&& color) {
    return Vec3f(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
}



int scene_1() {

    // CAMERA = Vec3(3.4589443, 2.62217, 12.626352) 0.20000002 -0.19999999
    // CAMERA = Vec3(0, 2.8092508, 14.059006) 0.20000002 0
    // Camera cam(45, Vec3(0.0f, 2.2026611f, 4.0398674f), 0.4f, 0.3f, 800, 600);
    // Camera cam(45, Vec3f(3.4589443f, 2.62217f, 12.626352f), 0.2f, -0.2f, 1280, 720);
     
    Camera cam(45, Vec3f(0.046539098f, 1.4768682f, 6.134056f), 0.1f, 0, 800, 600);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    // scene.add_object(Sphere(
    //     Vec3f(10.f,  10.f, 10.f),
    //     5.0f,
    //     Material{
    //         .albedo = Vec3f(1.0f),
    //         .roughness = 0.3f,
    //         .metallic = 0.0f,
    //         .emission_power = 120.0f,
    //     })
    // );

    scene.add_object(Sphere(
        Vec3f(-.5f,  0.5f, 0.f),
        0.4f,
        Material({
            .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
            .roughness = 0.0f,
            .metallic = 0.f,
        }))
    );

    // scene.add_object(Sphere(
    //     Vec3f(-0.0f,  .5f, 0.f),
    //     0.2f,
    //     Material({
    //         .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
    //         .roughness = 0.5f,
    //         .metallic = 0.f,
    //     }))
    // );

    scene.add_object(Sphere(
        Vec3f(.5f,  0.5f, 0.f),
        0.4f,
        Material({
            .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
            .roughness = 0.1f,
            .metallic = 1.f,
        }))
    );
    

    scene.add_object(Mesh(
        Vec3f(), 
        Material({ .albedo = Vec3(.9f, .9f, .9f), .emission_power = 50.0f }),
        load_obj("light.obj")
    ));

    scene.add_object(Box(
        Vec3f(-2, 1, 0),
        2, 2, 2, 0, 0, 0,
        Material({.albedo = Vec3f(1, 0, 0)})
    ));

    scene.add_object(Box(
        Vec3f(2, 1, 0),
        2, 2, 2, 0, 0, 0,
        Material({.albedo = Vec3f(0, 1, 0)})
    ));

    scene.add_object(Box(
        Vec3f(0, 1, -2),
        2, 2, 2, 0, 0, 0,
        Material({.albedo = Vec3f(1, 1, 1)})
    ));


    scene.add_object(Box(
        Vec3f(0, -1, 0),
        2, 2, 2, 0, 0, 0,
        Material({.albedo = Vec3f(1, 1, 1)})
    ));

    u32 selected_index = 0;
    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_1,
        .cb = [&selected_index]{
            fmt::println("0");
            selected_index = 0;
        }
    });


    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_2,
        .cb = [&selected_index]{
            fmt::println("1");
            selected_index = 1;
        }
    });


    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_3,
        .cb = [&selected_index, &scene, &cam]{
            Material& mat = scene.get_object<Sphere>(selected_index).m_material;
            mat.update_roughness(mat.roughness + 0.05f);
            cam.reset_accu_data();
        }
    });

    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_4,
        .cb = [&selected_index, &scene, &cam]{
            Material& mat = scene.get_object<Sphere>(selected_index).m_material;
            mat.update_roughness(mat.roughness - 0.05f);
            cam.reset_accu_data();
        }
    });


    while(!glfwWindowShouldClose(w.m_glfw_window)) {
        glfwPollEvents();
        scene.render(3);    

        if (w.framebuffer_resized) {
            r.recreate_swap_chain();
            w.framebuffer_resized = false;
            r.wait_for_device_idle();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        
        r.update_image(reinterpret_cast<u8*>(cam.image.data()));
        r.draw_frame();
    }
    
    r.wait_for_device_idle();
    return 0;
}



int scene_2() {
    Camera cam(45, Vec3(0.0f, 2.2026611f, 4.0398674f), 0.4, 0, 400, 300);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);
    scene.add_object(Mesh(
        Vec3f(), 
        Material({ .albedo = Vec3(.9f, .9f, .9f), .emission_power = 50.0f }),
        load_obj("light.obj")
    ));

    scene.add_object(Mesh(
        Vec3f(),
        Material({ 
            .albedo = u8_color_to_float(Vec3<u8>(200, 100, 25)),
            .roughness = 0.0f,
        }),
        load_obj("test.obj")
    ));

    scene.add_object(Mesh(
        Vec3f(),
        Material({
            .albedo = u8_color_to_float(Vec3<u8>(25, 200, 100)),
            .roughness = 0.3f,
            .metallic = 1.0f
        }),
        load_obj("suzanne.obj")
    ));
    
    {
        ScopedTimer<std::chrono::microseconds> s("Render");
        for (int i = 0; i < 10; i++) {
            ScopedTimer<std::chrono::milliseconds> s(fmt::format("iteration {}", i));
            scene.render(3);
        }
    }
    return 0;
    while (!glfwWindowShouldClose(w.m_glfw_window)) {
        glfwPollEvents();

        if (w.framebuffer_resized) {
            r.recreate_swap_chain();
            w.framebuffer_resized = false;
            r.wait_for_device_idle();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }


        r.update_image(reinterpret_cast<u8*>(cam.image.data()));
        r.draw_frame();
    }

    r.wait_for_device_idle();
    return 0;
}

int main() {
    return scene_1();
}