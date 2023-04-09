#pragma once
#include <string_view>
#include <vector>
#include <set>
#include <vulkan/vulkan.h>
#include <optional>
#include "window/window.hpp"

namespace renderer {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> presentation;
    
    bool good() {
        return (
            graphics.has_value() && presentation.has_value()
        );
    }
};
struct Queues {
    VkQueue graphics;
    VkQueue presentation;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class Renderer {
    

    const std::vector<const char*> m_required_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice m_phy = VK_NULL_HANDLE;
    VkDevice m_device;
    QueueFamilyIndices m_queues_indices;
    Queues m_queues;
    VkSurfaceKHR m_surface;
    Window& m_window;


    VkSwapchainKHR m_swap_chain;

    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;
    std::vector<VkImage> m_swap_chain_images;
    std::vector<VkImageView> m_swap_chain_image_views;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;

    VkRenderPass m_render_pass;
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_pipeline;
    VkCommandPool m_command_pool;
    VkCommandBuffer m_command_buffer;

    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_finished_semaphore;
    VkFence m_in_flight_fence;

    void create_instance();
    void setupDebugMessenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swap_chain();
    void create_image_views();
    void create_render_pass();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffer();
    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void create_sync_objects();

    std::vector<char> read_shader(std::string_view file_path);
    VkShaderModule create_shader_module(const std::vector<char>& code);
    QueueFamilyIndices find_queue_families(const VkPhysicalDevice& device);
    bool check_device_extensions_support(const VkPhysicalDevice& device);
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);

public:
    Renderer(Window& window);
    void draw_frame();
    void wait_for_device_idle();

    ~Renderer();

};

}

