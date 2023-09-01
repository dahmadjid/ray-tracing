#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Window {
public:
    GLFWwindow* m_window;
    Window();
     __attribute__((always_inline)) inline int should_close(){
        return glfwWindowShouldClose(m_window);
    }

    ~Window();
};


