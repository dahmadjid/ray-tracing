#pragma once
#include "ray-tracing/Camera.hpp"
#include <cstdint>
#include <fmt/core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


template<u32 window_width, u32 window_height>
class Window {
public:
    GLFWwindow* m_glfw_window;
    RayTracer::Camera<window_width, window_height>& cam; 
    
    
    Window(RayTracer::Camera<window_width, window_height>& cam): cam(cam) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_glfw_window = glfwCreateWindow(window_width, window_height, "Vulkan window", nullptr, nullptr);
        
        glfwSetWindowUserPointer(m_glfw_window, this);
        glfwSetKeyCallback(m_glfw_window, Window::key_callback);


    };

    __attribute__((always_inline)) inline int should_close(){
        return glfwWindowShouldClose(m_glfw_window);
    }

    ~Window() {
        glfwDestroyWindow(m_glfw_window);
        glfwTerminate();
    };

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Window* this_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

        f32 pitch = 0;
        f32 yaw = 0;
        switch(key) {
            case GLFW_KEY_DOWN:
                pitch += 0.1f;
                break;

            case GLFW_KEY_UP:
                pitch -= 0.1f;
                break;

            case GLFW_KEY_RIGHT:
                yaw += 0.1f;
                break;
            
            case GLFW_KEY_LEFT:
                yaw -= 0.1f;
                break;
            
            case GLFW_KEY_W:
                this_window->cam.update_z_position(0.5f);
                break;

            case GLFW_KEY_S:
                this_window->cam.update_z_position(-0.5f);
                break;

            case GLFW_KEY_D:
                this_window->cam.update_x_position(0.5f);
                break;

            case GLFW_KEY_A:
                this_window->cam.update_x_position(-0.5f);
                break;

            case GLFW_KEY_Q:
                this_window->cam.update_y_position(0.5f);
                break;

            case GLFW_KEY_E:
                this_window->cam.update_y_position(-0.5f);
                break;
        }

        if (pitch != 0 || yaw != 0) {
            this_window->cam.rotate(pitch, yaw);
            this_window->cam.calculate_ray_directions();
        }

    };


};


