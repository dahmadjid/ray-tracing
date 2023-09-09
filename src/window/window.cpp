#include "window.hpp"
#include <cstdint>


Window::Window(uint32_t width, uint32_t height) 
: m_width(width), m_height(height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_glfw_window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
}


Window::~Window() {
    glfwDestroyWindow(m_glfw_window);
    glfwTerminate();
}

