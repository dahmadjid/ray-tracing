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

    Camera<1920, 1080> cam(45);

    Window w(cam);
    auto r = renderer::Renderer(w);
    Scene scene(cam);

    scene.add_object(Sphere(
        Vec3<f32>(0.f,  0.f, 0.f),
        1.0f,
        Material{
            .albedo = Vec4<u8>(200, 0, 0, 255),
            .roughness = .8f,
            .metalic = 0.0f
        })
    );

    scene.add_object(Sphere(
        Vec3<f32>(0.f, -21.f, -3.f),
        20.0f, 
        Material{
            .albedo = Vec4<u8>(200, 200, 0, 255),
            .roughness = 0.f,
            .metalic = 0.0f
        })
    );


    scene.add_light(PointLight(Vec3<f32>(0.3f, 1.7f, 0), Vec4<u8>(255, 255, 255, 255)));
    

    // Ray shadow_ray;
    // shadow_ray.origin = payload->hit_position;
    // shadow_ray.direction = (light.position - shadow_ray.origin);
    // auto p = m_objects.any_hit(shadow_ray, 0.001f, std::numeric_limits<f32>::max());               
    // if (p.has_value()) {
    //     return -1.f;
    // } else {
    //     return std::max(payload->normal.dot(shadow_ray.direction), 0.0f);  // == cos(angle)
    // }
            
    
    while(!w.should_close()) {
        scene.render();
        r.update_image(reinterpret_cast<u8*>(cam.image->data()));
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();


}
