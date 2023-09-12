#include <chrono>
#include <fmt/core.h>
#include "ray-tracing/Ray.hpp"
#include "utils/ScopedTimer.hpp"
#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "stb_include.h"
#include "utils/Panic.hpp"

#include "ray-tracing/objects.hpp"

int main() {
    // image width and height needs to be the same as window size
    // window is not resizable

    // Window w(800, 600);
    // auto r = renderer::Renderer(w);
    
    // int width, height, channels;
    // stbi_uc* image = stbi_load("logo.png", &width, &height, &channels, STBI_rgb_alpha);
    // if (!image) {
    //     panic("Failed to read image");
    // }
    // while(!w.should_close()) {
    //     r.update_image(image);
    //     {
    //         ScopedTimer<std::chrono::microseconds, false> s("");
    //         r.draw_frame();
    //     }
    //     glfwPollEvents();
    // }
    
    // r.wait_for_device_idle();

    RayTracer::ObjectsList list;
    list.add_object(RayTracer::Sphere{});
    list.add_object(RayTracer::Sphere{});
    list.add_object(RayTracer::Sphere{});
    list.add_object(RayTracer::Box{});
    list.add_object(RayTracer::Box{});

    std::vector<RayTracer::HitPayload> payloads = list.hit(RayTracer::Ray{});

    for (const auto& p: payloads) {
        fmt::println("{}", p.t);
    }

}
