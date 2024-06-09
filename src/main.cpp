#include <fmt/core.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <thread>

#include "linear_algebra/ONB.hpp"
#include "linear_algebra/Vec3.decl.hpp"
#include "linear_algebra/Vec3.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Material.hpp"
#include "ray-tracing/Scene.hpp"
#include "ray-tracing/objects.hpp"
#include "renderer/renderer.hpp"
#include "utils/BMP.hpp"
#include "utils/Obj.hpp"
#include "window/window.hpp"

using namespace RayTracer;

constexpr Vec3f u8_color_to_float(Vec3<u8>&& color) {
    return Vec3f(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
}

int main() {
    Camera cam(45, Vec3f(0.046539098f, -0.042931885f, 5.7400503f), 0, 0, 1280, 1280);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);
    scene.add_object(
        Mesh(
            Vec3f(), 
            Material({
                .type = MaterialType::EMISSIVE, 
                .albedo = Vec3(1.0f, 1.0f, 1.0f), 
                .emission_power = 2.5f}
            ), 
            load_obj("light.obj")
        )
    );

    //scene.add_object(Sphere(
    //    Vec3f(-.5f, -0.59f, 0.f), 0.4f,
    //    Material({
    //        .type = MaterialType::METAL,
    //        .albedo = u8_color_to_float(Vec3<u8>(218, 165, 32)),
    //        .roughness = 0.05f,
    //    })
    //));

    //scene.add_object(Sphere(
    //    Vec3f(.5f, -0.59f, 0.f), 0.4f,
    //    Material({
    //        .type = MaterialType::METAL,
    //        .albedo = u8_color_to_float(Vec3<u8>(218, 165, 32)),
    //        .roughness = 0.05f,
    //    })
    //));
    scene.add_object(Mesh(
        Vec3f(),
        Material({
            .type = MaterialType::METAL,
            .albedo = u8_color_to_float(Vec3<u8>(255)),
            .roughness = 0.0f,
        }),
        load_obj("cube1.obj")
    ));

    scene.add_object(Mesh(
        Vec3f(),
        Material({
            .type = MaterialType::LAMBERTIAN,
            .albedo = u8_color_to_float(Vec3<u8>(218, 165, 32)),
        }),
        load_obj("cube2.obj")
    ));
    scene.add_object(Mesh(Vec3f(), Material({.albedo = Vec3f(1, 0, 0)}), load_obj("left.obj")));

    scene.add_object(Mesh(Vec3f(), Material({.albedo = Vec3f(0, 1, 0)}), load_obj("right.obj")));

    scene.add_object(Mesh(Vec3f(), Material({.albedo = Vec3f(1, 1, 1)}), load_obj("floor.obj")));

    scene.add_object(Mesh(Vec3f(), Material({.albedo = Vec3f(1, 1, 1)}), load_obj("back.obj")));

    u32 selected_index = 1;
    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_1,
        .cb =
            [&selected_index] {
                fmt::println("0");
                selected_index = 1;
            },
    });

    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_2,
        .cb =
            [&selected_index] {
                fmt::println("1");
                selected_index = 2;
            },
    });

    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_3,
        .cb =
            [&selected_index, &scene, &cam] {
                Material& mat = scene.get_object<Sphere>(selected_index).m_material;
                mat.update_roughness(mat.roughness + 0.05f);
                cam.reset_accu_data();
            },
    });

    w.custom_key_cbs.push_back(CustomKeyCallback{
        .key = GLFW_KEY_4,
        .cb =
            [&selected_index, &scene, &cam] {
                Material& mat = scene.get_object<Sphere>(selected_index).m_material;
                mat.update_roughness(mat.roughness - 0.05f);
                cam.reset_accu_data();
            },
    });
    u32 spp = 4096;

    while (!glfwWindowShouldClose(w.m_glfw_window)) {
        glfwPollEvents();
        if (scene.m_camera.frame_index < spp) {
            scene.render(8);
        } else {
            write_bmp_image("D:\\render.bmp", cam.image, cam.window_width, cam.window_height);
            std::terminate();
        }
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
