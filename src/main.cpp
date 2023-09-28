#include <fmt/core.h>
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

    Window w(1280, 720);
    auto r = renderer::Renderer(w);
    Camera<1280, 720> cam(45);
    Scene<1280, 720> scene(cam);


    scene.add_object(Sphere(Vec3<float>(0., 0., 0.), 0.5f, Vec4<uint8_t>(255, 0, 0, 255)));
    scene.add_object(Sphere(Vec3<float>(0., -100.5, 0.), 100.f, Vec4<uint8_t>(0, 255, 0, 255)));
    auto image = new std::array<Vec4<uint8_t>, 1280 * 720>();
    while(!w.should_close()) {
        scene.render(*image);
        r.update_image(reinterpret_cast<uint8_t*>(image->data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();


}
