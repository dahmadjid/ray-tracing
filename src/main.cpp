#include <chrono>
#include <cstdint>
#include <fmt/core.h>
#include "linear_algebra/Vec3.decl.hpp"
#include "ray-tracing/Ray.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "stb_include.h"
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

   
    while(!w.should_close()) {
        r.update_image(reinterpret_cast<uint8_t*>(scene.render().data()));
        {
            ScopedTimer<std::chrono::microseconds, false> s("");
            r.draw_frame();
        }
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();

    // RayTracer::ObjectsList list;
    // list.add_object(RayTracer::Sphere(Vec3(0.5f, 0.5f, 0.5f), 1., Vec3(0.5f, 0.5f, 0.5f)));
    // list.add_object(RayTracer::Sphere(Vec3(0.5f, 0.5f, 0.5f), 1., Vec3(0.5f, 0.5f, 0.5f)));
    // list.add_object(RayTracer::Sphere(Vec3(0.5f, 0.5f, 0.5f), 1., Vec3(0.5f, 0.5f, 0.5f)));

    // auto payloads = list.hit(RayTracer::Ray{.origin=Vec3(1.0f, 1.0f, 1.0f), .direction=Vec3(1.0f, 1.0f, 1.0f)}, 0.00001 , 5.2);

    // for (const auto& p: payloads) {
    //     fmt::println("{}", p->t);
    // }

}
