#include <fmt/core.h>
#include <memory>
#include "ray-tracing/Ray.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.impl.hpp"
#include "utils/Panic.hpp"
#include "ray-tracing/Camera.hpp"
#include "ray-tracing/Scene.hpp"
#include "ray-tracing/objects.hpp"

using namespace RayTracer;





int main() {
    // image width and height needs to be the same as window size
    // window is not resizable

    Camera<1280, 720> cam(45);

    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    // scene.add_object(Sphere(Vec3<f32>(0, 1, -3), 0.3f, Vec4<u8>(0, 0, 127, 255)));
    // scene.add_object(Sphere(Vec3<f32>(0., 0., -3), 0.5f, Vec4<u8>(0, 127, 0, 255)));
    // scene.add_object(Sphere(Vec3<f32>(0, -1,-3), 0.5f, Vec4<u8>(127, 0, 0, 255)));
    scene.add_object(Sphere(Vec3<f32>(0, -8.f, -3.2), 8.f, Vec4<u8>(127, 0, 0, 255)));
    scene.add_light(PointLight(Vec3<f32>(0, 2, -3), Vec4<u8>(255, 255, 255, 255)));

            
    auto image = std::make_unique<std::array<Vec4<u8>, 1280 * 720>>();
    while(!w.should_close()) {
        scene.render(*image);
        r.update_image(reinterpret_cast<u8*>(image->data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();


}
