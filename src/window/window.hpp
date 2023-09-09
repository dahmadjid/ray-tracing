#pragma once
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Window {
public:
    GLFWwindow* m_glfw_window;
    uint32_t m_width;
    uint32_t m_height;
    
    Window(uint32_t width, uint32_t height);

    __attribute__((always_inline)) inline int should_close(){
        return glfwWindowShouldClose(m_glfw_window);
    }

    ~Window();
};


