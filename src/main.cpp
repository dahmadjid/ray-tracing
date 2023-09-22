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

    Window w(800, 600);
    auto r = renderer::Renderer(w);
    Camera<800, 600> cam(45);
    Scene<800, 600> scene(cam);


    scene.add_object(Sphere(Vec3<float>(0., 0., 0.), 0.5f, Vec4<uint8_t>(255, 0, 0, 255)));
    scene.add_object(Sphere(Vec3<float>(0., -100.5, 0.), 100.f, Vec4<uint8_t>(0, 255, 0, 255)));
    std::array<Vec4<uint8_t>, 800 * 600> image;
    while(!w.should_close()) {
        scene.render(image);
        r.update_image(reinterpret_cast<uint8_t*>(image.data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();


}
