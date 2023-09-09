#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include <array>
#include <set>
#include <vulkan/vulkan.h>
#include <optional>
#include <vulkan/vulkan_core.h>
#include "window/window.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <stb_image.h>

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

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Renderer {


    const std::vector<const char*> m_required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const std::vector<const char *> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif
    const int MAX_FRAMES_IN_FLIGHT = 2;

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
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_pipeline;
    VkCommandPool m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;
    uint32_t m_current_frame = 0;
    
    VkBuffer m_vertex_buffer;
    VkDeviceMemory m_vertex_buffer_memory;

    VkBuffer m_index_buffer;
    VkDeviceMemory m_index_buffer_memory;

    std::vector<VkBuffer> m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffers_memory;
    std::vector<void*> m_uniform_buffers_mapped;
    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorSet> m_descriptor_sets;    

    VkImage m_texture_image;
    VkDeviceMemory m_texture_image_memory;
    void* m_texture_image_mapped;
    VkImageView m_texture_image_view;
    VkSampler m_texture_sampler;
    stbi_uc* m_pixels;
    VkDeviceSize m_image_size;
    int m_width;
    int m_height;
    int m_channels;


    const std::vector<Vertex> m_vertices = {
        {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> m_indices = {
        0, 1, 2, 2, 3, 0
    };

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
    void create_command_buffers();
    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void create_sync_objects();
    void recreate_swap_chain();
    void cleanup_swap_chain();  

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    void create_vertex_buffer();
    void create_index_buffer();
    void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void create_descriptor_set_layout();
    void create_descriptor_pool();
    void create_descriptor_sets();
    void create_uniform_buffers();
    void update_uniform_buffers(uint32_t current_frame);
    void create_texture_image();
    void create_image(
        uint32_t width, 
        uint32_t height, 
        VkFormat format, 
        VkImageTiling tiling, 
        VkImageUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        VkImage& image, 
        VkDeviceMemory& image_memory
    );
    void create_texture_sampler();

    uint32_t find_memory_types(uint32_t type_filter, VkMemoryPropertyFlags properties);
    std::vector<char> read_shader(std::string_view file_path);
    VkShaderModule create_shader_module(const std::vector<char>& code);
    QueueFamilyIndices find_queue_families(const VkPhysicalDevice& device);
    bool check_device_extensions_support(const VkPhysicalDevice& device);
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer command_buffer);
    void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void create_texture_image_view(); 
    void write_image();
public:
    Renderer(Window& window);
    void draw_frame();
    void wait_for_device_idle();

    ~Renderer();

};

}

