#pragma once
#include "ray-tracing/Camera.hpp"
#include "utils/BMP.hpp"
#include <cstdint>
#include <fmt/core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct FloatKeyControl {
    int increment_glfw_key;
    int decrement_glfw_key;
    float* variable_to_change;
    float change_step;
    float min = 0.0f;
    float max = 1.0f;
};

class Window {
public:
    GLFWwindow* m_glfw_window;
    RayTracer::Camera& cam; 
    bool framebuffer_resized = false;
    std::vector<FloatKeyControl> float_key_controls;


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
        static f32 all_pitch_changes = 0;
        static f32 all_yaw_changes = 0;
        
        f32 pitch = 0;
        f32 yaw = 0;
        switch(key) {
            case GLFW_KEY_C:
                fmt::println("CAMERA = {} {} {}", this_window->cam.position(), all_pitch_changes, all_yaw_changes);
                break;

            case GLFW_KEY_V:
                write_bmp_image("render.bmp", this_window->cam.image, this_window->cam.window_width, this_window->cam.window_height);
                break;
            case GLFW_KEY_DOWN:
                pitch += 0.05f;
                break;

            case GLFW_KEY_UP:
                pitch -= 0.05f;
                break;

            case GLFW_KEY_RIGHT:
                yaw += 0.05f;
                break;
            
            case GLFW_KEY_LEFT:
                yaw -= 0.05f;
                break;
            
            case GLFW_KEY_W:
                this_window->cam.update_z_position(0.2f);
                break;

            case GLFW_KEY_S:
                this_window->cam.update_z_position(-0.2f);
                break;

            case GLFW_KEY_D:
                this_window->cam.update_x_position(0.2f);
                break;

            case GLFW_KEY_A:
                this_window->cam.update_x_position(-0.2f);
                break;

            case GLFW_KEY_Q:
                this_window->cam.update_y_position(0.2f);
                break;

            case GLFW_KEY_E:
                this_window->cam.update_y_position(-0.2f);
                break;

            default:
                for (const auto& key_control: this_window->float_key_controls) {
                    if (key_control.increment_glfw_key == key) {
                        if ((*key_control.variable_to_change) < key_control.max) {
                            auto res = (*key_control.variable_to_change) + key_control.change_step;
                            (*key_control.variable_to_change) = std::min(res, key_control.max);
                            this_window->cam.reset_accu_data();
                            fmt::println("{}", (*key_control.variable_to_change));
                            break;
                        } 
                    } else if (key_control.decrement_glfw_key == key) {
                        if ((*key_control.variable_to_change) > key_control.min) {
                            auto res = (*key_control.variable_to_change) - key_control.change_step;
                            (*key_control.variable_to_change) = std::max(res, key_control.min);
                            this_window->cam.reset_accu_data();
                            fmt::println("{}", (*key_control.variable_to_change));
                            break;
                        } 
                    }
                }
        }

        if (pitch != 0 || yaw != 0) {
            all_pitch_changes += pitch;
            all_yaw_changes += yaw;
            this_window->cam.rotate(pitch, yaw);
            this_window->cam.calculate_ray_directions();
        }

    };


};


