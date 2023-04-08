#include "window.hpp"


Window::Window() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}


Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

