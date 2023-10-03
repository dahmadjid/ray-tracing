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
    // image width and height needs to be the same as window size
    // window is not resizable

    Camera<1920, 1080> cam(45, Vec3(0.f, 0.f, 10.f), 0, 0);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    scene.add_object(Sphere(
        Vec3<f32>(4.f,  14.f, 4.f),
        10.0f,
        Material{
            .albedo = Vec3<f32>(1.0f),
            .roughness = 1.0f,
            .metalic = 0.0f,
            .emission_color = Vec3<f32>(1.0f),
            .emission_power = 1.0f
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(1.2f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(255, 20, 0)),
            .roughness = 1.0f,
            .metalic = 0.0f,
        })
    );


    scene.add_object(Sphere(
        Vec3<f32>(-1.2f, 0.f, 0.0f),
        1.0f,
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(25, 0, 200)),
            .roughness = .3f,
            .metalic = 0.0f,
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(0.f, -21.f, -0.f),
        20.0f, 
        Material{
            .albedo = u8_color_to_float(Vec3<u8>(100, 100, 100)),
            .roughness = 1.0f,
            .metalic = 0.0f
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
