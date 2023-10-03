#include <filesystem>
#include <fmt/core.h>
#include <memory>
#include "ray-tracing/Ray.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "utils/Panic.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Scene.hpp"
#include "ray-tracing/objects.hpp"

using namespace RayTracer;





int main() {
    // image width and height needs to be the same as window size
    // window is not resizable

    Camera<1920, 1080> cam(45, Vec3(20.f, 45.f, 50.f), 3.9f, 3.4f);
    cam.update_z_position(50);
    cam.update_x_position(-5);
    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    scene.add_object(Sphere(
        Vec3<f32>(0.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = Vec4<u8>(200, 0, 0, 255),
            .roughness = .0f,
            .metalic = 0.0f
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(0.f, -21.f, -3.f),
        20.0f, 
        Material{
            .albedo = Vec4<u8>(60, 120, 200, 255),
            .roughness = 0.1f,
            .metalic = 0.0f
        })
    );


    scene.add_light(PointLight(Vec3<f32>(10.f, 10.f, 10.f), Vec4<u8>(255, 255, 255, 255)));
    
    while(!w.should_close()) {
        scene.render(3);
        r.update_image(reinterpret_cast<u8*>(cam.image->data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();


}
