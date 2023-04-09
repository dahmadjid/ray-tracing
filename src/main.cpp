#include "window/window.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <fmt/core.h>
#include "renderer/renderer.hpp"
int main() {
    
    Window w;
    auto r = renderer::Renderer(w);
    while(!w.should_close()) {
        r.draw_frame();
        glfwPollEvents();
    }
    
    r.wait_for_device_idle();

}
