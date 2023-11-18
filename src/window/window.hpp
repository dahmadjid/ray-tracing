#pragma once
#include "ray-tracing/Camera.hpp"
#include <cstdint>
#include <fmt/core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Window {
public:
    GLFWwindow* m_glfw_window;
    RayTracer::Camera& cam; 
    
    bool framebuffer_resized = false;


    Window(RayTracer::Camera& cam): cam(cam) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_glfw_window = glfwCreateWindow((i32)cam.window_width, (i32)cam.window_height, "Vulkan", nullptr, nullptr);
        
        glfwSetWindowUserPointer(m_glfw_window, this);
        glfwSetKeyCallback(m_glfw_window, Window::key_callback);
        glfwSetFramebufferSizeCallback(m_glfw_window, this->frame_buffer_resize_event);
    };

    static void frame_buffer_resize_event(GLFWwindow* window, int width, int height) {
        Window* this_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        this_window->cam.resize_camera(width, height);
        this_window->framebuffer_resized = true;
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


