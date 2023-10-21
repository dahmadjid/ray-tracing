#include <cstdio>
#include <filesystem>
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



Vec3<f32> u8_color_to_float(Vec3<u8>&& color) {
    return Vec3<f32>(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
}

int main() {
    // window is not resizable

    Camera<1920, 1080> cam(45, Vec3(0.f, 0.f, 10.f), 0, 0);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    scene.add_object(Sphere(
        Vec3<f32>(25.f,  25.f, 25.f),
        25.0f,
        Material{
            .albedo = Vec3<f32>(1.0f),
            .roughness = 1.0f,
            .metalic = 0.1f,
            .emission_power = 20.0f,
        })
    );


    scene.add_object(Sphere(
        Vec3<f32>(1.1f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(25, 0, 200)),
            .roughness = .0f,
            .metalic = 1.f,
        })
    );


    scene.add_object(Sphere(
        Vec3<f32>(-1.1f, 0.f, 0.0f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(25, 0, 200)),
            .roughness = 0.5f,
            .metalic = 0.0f,
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(0.f, -21.f, -0.f),
        20.0f, 
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(170, 180, 190)),
            .roughness = 1.0f,
            .metalic = 0.0f,
        })
    );

    while(!w.should_close()) {
        scene.render(10);
        r.update_image(reinterpret_cast<u8*>(cam.image->data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();

}
