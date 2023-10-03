#include "renderer.decl.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <optional>
#include <ratio>
#include <vector>
#include <set>
#include <fmt/format.h>
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan_core.h>
#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/Panic.hpp"

namespace renderer
{
    namespace chrono = std::chrono;

    template<u32 window_width, u32 window_height>
    Renderer<window_width, window_height>::Renderer(Window<window_width, window_height> &window) : m_window(window) {
        create_instance();
        setupDebugMessenger();
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_views();
        create_render_pass();
        create_descriptor_set_layout();
        create_graphics_pipeline();
        create_framebuffers();
        create_command_pool();
        create_texture_image();
        create_texture_image_view();
        create_texture_sampler();
        create_vertex_buffer();
        create_index_buffer();
        create_uniform_buffers();
        create_descriptor_pool();
        create_descriptor_sets();
        create_command_buffers();
        create_sync_objects();
        
    }



    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_instance() {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
        bool validation_supported = true;
        for (const char *layer_name : validation_layers) {
            bool layer_found = false;
            for (const auto &layer_propreties : available_layers)
            {
                if (strcmp(layer_name, layer_propreties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                validation_supported = false;
            }
        }

        if (!validation_supported && enable_validation) {
            panic("Validation layers requested, but not available!");

        }
        else {
            fmt::println("validation layers on");
        }
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enable_validation) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        create_info.enabledExtensionCount = extensions.size();
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            create_info.pNext = &debug_create_info;
        }
        else {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;

        }
        auto res = vkCreateInstance(&create_info, nullptr, &m_instance);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE VK INSTANCE: {}", string_VkResult(res));

        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_surface() {
        auto res = glfwCreateWindowSurface(m_instance, m_window.m_glfw_window, nullptr, &m_surface);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE VK SURFACE: {}", string_VkResult(res));

        }
        fmt::println("Surface created successfully");
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::pick_physical_device() {
        uint32_t devices_count = 0;
        auto res = vkEnumeratePhysicalDevices(m_instance, &devices_count, NULL);
        if (res != VK_SUCCESS && res != VK_INCOMPLETE) {
            panic("FAILED TO ENUMERATE VK PHYSICAL DEVICES: {}", string_VkResult(res));

        }
        std::vector<VkPhysicalDevice> devices(devices_count);

        if (devices_count == 0) {
            panic("NO PHYSICAL DEVICES FOUND");

        }

        vkEnumeratePhysicalDevices(m_instance, &devices_count, devices.data());
        for (const auto &device : devices) {
            auto queues = find_queue_families(device);
            bool extension_supported = check_device_extensions_support(device);
            VkPhysicalDeviceFeatures supported_features;
            vkGetPhysicalDeviceFeatures(device, &supported_features);
            if (extension_supported) {
                auto chain_support = query_swap_chain_support(device);
                if (
                    queues.good() && 
                    !chain_support.formats.empty() && 
                    !chain_support.present_modes.empty() &&
                    supported_features.logicOp &&
                    supported_features.samplerAnisotropy
                ) {
                    m_phy = device;
                    m_queues_indices = queues;
                    break;
                }
            }
        }
        if (m_phy == VK_NULL_HANDLE) {
            panic("NO SUITABLE PHYSICAL DEVICES FOUND");

        }
        fmt::println("Found {} GPU", devices_count);
    }
    template<u32 window_width, u32 window_height>
    QueueFamilyIndices Renderer<window_width, window_height>::find_queue_families(const VkPhysicalDevice &device) {

        uint32_t queues_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, nullptr);

        std::vector<VkQueueFamilyProperties> queues(queues_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, queues.data());
        QueueFamilyIndices queues_indices_temp;
        int i = 0;
        VkBool32 present_support = false;
        for (const auto &queue : queues) {
            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queues_indices_temp.graphics = i;
            }
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);
            if (present_support)
            {
                queues_indices_temp.presentation = i;
            }
            if (queues_indices_temp.good())
            {
                break;
            }
            i++;
        }
        return queues_indices_temp;
    }
    template<u32 window_width, u32 window_height>
    bool Renderer<window_width, window_height>::check_device_extensions_support(const VkPhysicalDevice &device) {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available_extensions.data());
        
        int required_found = 0;
        for (const auto &extension : available_extensions) {
            for (const auto& req: m_required_extensions) {
                if (strcmp(req, extension.extensionName) == 0) {
                    required_found += 1;
                }
            }
        }

        return m_required_extensions.size() == required_found;
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_logical_device() {
        auto queue_families = std::set({m_queues_indices.graphics.value(), m_queues_indices.presentation.value()});
        float prio = 1.0f;
        auto queues_create_infos = std::vector<VkDeviceQueueCreateInfo>();
        for (const auto &family_index : queue_families) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = family_index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &prio;
            queues_create_infos.emplace_back(queue_create_info);
        }
        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;
        device_features.logicOp = VK_TRUE;
        VkDeviceCreateInfo create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pEnabledFeatures = &device_features;

        create_info.pQueueCreateInfos = queues_create_infos.data();
        create_info.queueCreateInfoCount = queues_create_infos.size();

        create_info.enabledExtensionCount = m_required_extensions.size();

        create_info.ppEnabledExtensionNames = m_required_extensions.data();

        auto res = vkCreateDevice(m_phy, &create_info, nullptr, &m_device);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE VK LOGICAL DEVICES: {}", string_VkResult(res));

        }
        vkGetDeviceQueue(m_device, m_queues_indices.graphics.value(), 0, &m_queues.graphics);
        vkGetDeviceQueue(m_device, m_queues_indices.presentation.value(), 0, &m_queues.presentation);
        fmt::println("Logical Device created successfully");
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_swap_chain() {
        auto details = query_swap_chain_support(m_phy);
        auto selected_format = details.formats[0];
        auto selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        auto actual_extent = details.capabilities.currentExtent;

        for (const auto &format : details.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                selected_format = format;
            }
        }

        for (const auto &present_mode : details.present_modes) {
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
        if (selected_format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            fmt::println("Selected Image Format: VK_FORMAT_B8G8R8A8_SRGB");
        }
        else {
            fmt::println("Selected Image Format: {}", static_cast<int>(selected_format.format));
        }
        if (selected_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            fmt::println("Selected Presentation mode: VK_PRESENT_MODE_MAILBOX_KHR");
        }
        else {
            fmt::println("Selected Presentation mode: {}", static_cast<int>(selected_present_mode));
        }

        if (details.capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            int width, height;
            glfwGetFramebufferSize(m_window.m_glfw_window, &width, &height);
            actual_extent.width = std::clamp((uint32_t)width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp((uint32_t)height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
        }

        uint32_t image_count = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
            image_count = details.capabilities.maxImageCount;
        }

        fmt::println("Extent Width: {}, Extent Height: {}, Image Count: {}", actual_extent.width, actual_extent.height, image_count);

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.pNext = NULL;
        create_info.surface = m_surface;
        create_info.minImageCount = image_count;
        create_info.imageFormat = selected_format.format;
        create_info.imageColorSpace = selected_format.colorSpace;
        create_info.imageExtent = actual_extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queue_indices[] = {m_queues_indices.graphics.value(), m_queues_indices.presentation.value()};
        if (m_queues_indices.graphics != m_queues_indices.presentation) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_indices;
            fmt::println("Image Sharing Mode: VK_SHARING_MODE_CONCURRENT");

        } else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = NULL;
            fmt::println("Image Sharing Mode: VK_SHARING_MODE_EXCLUSIVE");

        }
        create_info.preTransform = details.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = selected_present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;
        auto res = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swap_chain);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE SWAP CHAIN: {}", string_VkResult(res));

        }

        fmt::println("Swap chain created successfully");

        vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
        m_swap_chain_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_chain_images.data());
        m_swap_chain_image_format = selected_format.format;
        m_swap_chain_extent = actual_extent;
    }
    template<u32 window_width, u32 window_height>
    SwapChainSupportDetails Renderer<window_width, window_height>::query_swap_chain_support(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
        }
        uint32_t present_modes_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, nullptr);

        if (present_modes_count != 0) {
            details.present_modes.resize(present_modes_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, details.present_modes.data());
        }

        return details;
    }

    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_image_views() {
        m_swap_chain_image_views.resize(m_swap_chain_images.size());
        for (size_t i = 0; i < m_swap_chain_image_views.size(); i++) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = m_swap_chain_images[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = m_swap_chain_image_format;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;
            auto res = vkCreateImageView(m_device, &create_info, nullptr, &m_swap_chain_image_views[i]);
            if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE IMAGE VIEW: {}", string_VkResult(res));


            }
        }
    }
    template<u32 window_width, u32 window_height>
    VkShaderModule Renderer<window_width, window_height>::create_shader_module(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        auto res = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE SHADER MODULE: {}", string_VkResult(res));

        }
        return shaderModule;

    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_render_pass() {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = m_swap_chain_image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;
        auto res = vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE RENDER PASS: {}", string_VkResult(res));

        }

    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_graphics_pipeline() {
        auto vertex_shader = read_shader("shaders/vertex.spv");
        auto fragment_shader = read_shader("shaders/fragment.spv");
        auto vert_shader_module = create_shader_module(vertex_shader);
        auto frag_shader_module = create_shader_module(fragment_shader);


        VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
        vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_info.module = vert_shader_module;
        vertex_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
        frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = frag_shader_module;
        frag_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info, frag_shader_stage_info};



        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
        vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();


        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;


        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_swap_chain_extent.width;
        viewport.height = (float) m_swap_chain_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swap_chain_extent;


        std::vector<VkDynamicState> dynamic_states_wanted = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states_wanted.size());
        dynamic_state.pDynamicStates = dynamic_states_wanted.data();

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        
        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_TRUE;
        color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f; // Optional
        color_blending.blendConstants[1] = 0.0f; // Optional
        color_blending.blendConstants[2] = 0.0f; // Optional
        color_blending.blendConstants[3] = 0.0f; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout;

        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        auto res = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipeline_layout);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE PIPELINE LAYOUT: {}", string_VkResult(res));
        }


        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = 2;
        pipeline_create_info.pStages = shader_stages;

        pipeline_create_info.pVertexInputState = &vertex_input_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly;
        pipeline_create_info.pViewportState = &viewport_state;
        pipeline_create_info.pRasterizationState = &rasterizer;
        pipeline_create_info.pMultisampleState = &multisampling;
        pipeline_create_info.pDepthStencilState = nullptr; // Optional
        pipeline_create_info.pColorBlendState = &color_blending;
        pipeline_create_info.pDynamicState = &dynamic_state;

        pipeline_create_info.layout = m_pipeline_layout;
        pipeline_create_info.renderPass = m_render_pass;
        pipeline_create_info.subpass = 0;

        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipeline_create_info.basePipelineIndex = -1; // Optional

        res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_pipeline);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE PIPELINE: {}", string_VkResult(res));

        }
        
        fmt::println("Graphics pipeline created successfully");
        vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_device, frag_shader_module, nullptr);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_framebuffers() {
        m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());
        for (size_t i = 0; i < m_swap_chain_image_views.size(); i++) {
            VkImageView attachments[] = {
                m_swap_chain_image_views[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_render_pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swap_chain_extent.width;
            framebufferInfo.height = m_swap_chain_extent.height;
            framebufferInfo.layers = 1;
            auto res = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swap_chain_framebuffers[i]);
            if (res != VK_SUCCESS) {
                panic("FAILED TO CREATE FRAMEBUFFER: {}", string_VkResult(res));
            }
        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_command_pool() {

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = m_queues_indices.graphics.value();
        auto res = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE COMMAND POOL: {}", string_VkResult(res));

        }
    }
        template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_command_buffers() {

        m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = m_command_pool;
        cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_alloc_info.commandBufferCount = m_command_buffers.size();

        auto res = vkAllocateCommandBuffers(m_device, &cmd_alloc_info, m_command_buffers.data());
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE COMMAND BUFFERS: {}", string_VkResult(res));

        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        auto res = vkBeginCommandBuffer(command_buffer, &beginInfo);
        if (res != VK_SUCCESS) {
            panic("FAILED TO BEGIN RECORDING COMMAND BUFFER: {}", string_VkResult(res));

        }

        if (m_image_updated) {
            write_image(command_buffer, m_current_frame);
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_render_pass;
        render_pass_info.framebuffer = m_swap_chain_framebuffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m_swap_chain_extent;
        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;


        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);


        VkBuffer vertex_buffers[] = {m_vertex_buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(command_buffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT16);


        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swap_chain_extent.width);
        viewport.height = static_cast<float>(m_swap_chain_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swap_chain_extent;
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(
            command_buffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_pipeline_layout, 
            0, 1, 
            &m_descriptor_sets[m_current_frame], 
            0, nullptr
        );

        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(command_buffer);
        res = vkEndCommandBuffer(command_buffer);
        if (res != VK_SUCCESS) {
            panic("FAILED TO RECORD COMMAND BUFFER: {}", string_VkResult(res));

        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_sync_objects() {

        m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (
                vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS
            ) {
            panic("FAILED TO CEATE SYNC OBJECTS");


            }
        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::recreate_swap_chain() {
        vkDeviceWaitIdle(m_device);
        cleanup_swap_chain();
        create_swap_chain();
        create_image_views();
        create_framebuffers();
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::cleanup_swap_chain() {
        for (auto framebuffer : m_swap_chain_framebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }

        for (auto image_view : m_swap_chain_image_views) {
            vkDestroyImageView(m_device, image_view, nullptr);
        }
        
        vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_buffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory
    ) {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        auto res = vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE BUFFER: {}", string_VkResult(res));

        }
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &mem_requirements);
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_types(mem_requirements.memoryTypeBits, properties);

        res = vkAllocateMemory(m_device, &alloc_info, nullptr, &buffer_memory);
        if (res != VK_SUCCESS) {
            panic("FAILED TO ALLOCATE MEMORY FOR BUFFER: {}", string_VkResult(res));

        }

        res = vkBindBufferMemory(m_device, buffer, buffer_memory, 0);
        if (res != VK_SUCCESS) {
            panic("FAILED TO BIND MEMORY TO BUFFER: {}", string_VkResult(res));

        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_vertex_buffer() {
        
        VkDeviceSize buffer_size = sizeof(m_vertices[0]) * m_vertices.size();


        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        create_buffer(
            buffer_size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            staging_buffer, 
            staging_buffer_memory
        );
        
        void* data;
        vkMapMemory(m_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, m_vertices.data(), (size_t) buffer_size);
        vkUnmapMemory(m_device, staging_buffer_memory);

        create_buffer(
            buffer_size, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            m_vertex_buffer, 
            m_vertex_buffer_memory
        );

        copy_buffer(staging_buffer, m_vertex_buffer, buffer_size);
        
        vkDestroyBuffer(m_device, staging_buffer, nullptr);
        vkFreeMemory(m_device, staging_buffer_memory, nullptr);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_index_buffer() {
        
        VkDeviceSize buffer_size = sizeof(m_indices[0]) * m_indices.size();


        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        create_buffer(
            buffer_size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            staging_buffer, 
            staging_buffer_memory
        );
        
        void* data;
        vkMapMemory(m_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, m_indices.data(), (size_t) buffer_size);
        vkUnmapMemory(m_device, staging_buffer_memory);

        create_buffer(
            buffer_size, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            m_index_buffer, 
            m_index_buffer_memory
        );

        copy_buffer(staging_buffer, m_index_buffer, buffer_size);
        
        vkDestroyBuffer(m_device, staging_buffer, nullptr);
        vkFreeMemory(m_device, staging_buffer_memory, nullptr);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_uniform_buffers() {
        VkDeviceSize buffer_size = sizeof(UniformBufferObject);

        m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            create_buffer(
                buffer_size, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                m_uniform_buffers[i], 
                m_uniform_buffers_memory[i]
            );

            vkMapMemory(m_device, m_uniform_buffers_memory[i], 0, buffer_size, 0, &m_uniform_buffers_mapped[i]);
        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
        auto command_buffer = begin_single_time_commands();

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);
        
        end_single_time_commands(command_buffer);
    }

    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_descriptor_set_layout() {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding sampler_layout_binding{};
        sampler_layout_binding.binding = 1;
        sampler_layout_binding.descriptorCount = 1;
        sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout_binding.pImmutableSamplers = nullptr;
        sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();

        auto res = vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &m_descriptor_set_layout);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE DESCRIPTOR SET LAYOUT");

        }

    }
    template<u32 window_width, u32 window_height>
    uint32_t Renderer<window_width, window_height>::find_memory_types(uint32_t type_filter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(m_phy, &mem_properties);
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        panic("FAILED TO FIND MEMORY TYPE FOR THE BUFFER");
        return 0;
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::draw_frame() {
        vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

        uint32_t image_index;
        auto res = vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            recreate_swap_chain();
            return;
        } else if (res != VK_SUCCESS) {
            panic("FAILED TO ACQUIRE NEXT IMAGE KHR: {}", string_VkResult(res));

        }
        
        // update_uniform_buffers(m_current_frame);
        vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
        record_command_buffer(m_command_buffers[m_current_frame], image_index);
    
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {m_image_available_semaphores[m_current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = waitSemaphores;
        submit_info.pWaitDstStageMask = waitStages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];

        VkSemaphore signalSemaphores[] = {m_render_finished_semaphores[m_current_frame]};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signalSemaphores;

        res = vkQueueSubmit(m_queues.graphics, 1, &submit_info, m_in_flight_fences[m_current_frame]);
        if (res != VK_SUCCESS) {
            panic("FAILED TO SUBMIT COMMAND BUFFER: {}", string_VkResult(res));

        }
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {m_swap_chain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapChains;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr; // Optional
        res = vkQueuePresentKHR(m_queues.presentation, &present_info);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            recreate_swap_chain();
            return;
        } else if (res != VK_SUCCESS) {
            panic("FAILED TO ACQUIRE NEXT IMAGE KHR: {}", string_VkResult(res));
        }
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_descriptor_pool() {
        std::array<VkDescriptorPoolSize, 2> pool_sizes{};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        
        
        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        auto res = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptor_pool);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO CREATE DESCRIPTOR POOL: {}", string_VkResult(res));
        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_descriptor_sets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptor_set_layout);
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        alloc_info.pSetLayouts = layouts.data();
        m_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
        auto res = vkAllocateDescriptorSets(m_device, &alloc_info, m_descriptor_sets.data());
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE DESCRIPTOR SETS: {}", string_VkResult(res));
        }
        std::vector<VkWriteDescriptorSet> all_descriptor_writes;
        all_descriptor_writes.reserve(MAX_FRAMES_IN_FLIGHT * 2);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            std::vector<VkWriteDescriptorSet> descriptor_writes;
            descriptor_writes.resize(2);

            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = m_uniform_buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = m_texture_image_view;
            image_info.sampler = m_texture_sampler;

            descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[0].dstSet = m_descriptor_sets[i];
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].dstArrayElement = 0;
            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].descriptorCount = 1;
            descriptor_writes[0].pBufferInfo = &buffer_info;


            descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[1].dstSet = m_descriptor_sets[i];
            descriptor_writes[1].dstBinding = 1;
            descriptor_writes[1].dstArrayElement = 0;
            descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[1].descriptorCount = 1;
            descriptor_writes[1].pImageInfo = &image_info;

            for (auto desc: descriptor_writes) {
                all_descriptor_writes.push_back(desc);
            }
        }
        vkUpdateDescriptorSets(m_device, all_descriptor_writes.size(), all_descriptor_writes.data(), 0, nullptr);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::setupDebugMessenger() {
        if (!enable_validation)
            return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        auto res = CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger);
        if (res != VK_SUCCESS){
            panic("FAILED TO ENUMERATE VK PHYSICAL DEVICES: {}", string_VkResult(res));

        }

    }
    template<u32 window_width, u32 window_height>
    std::vector<char> Renderer<window_width, window_height>::read_shader(std::string_view file_path) {
        std::ifstream stream;
        stream.open(file_path.data(), std::ios_base::ate | std::ios_base::binary);
        if (!stream.good()) {
            fmt::println("Failure reading file {}", file_path);
            return {};
        }
        auto size = stream.tellg();
        std::vector<char> buffer(size);
        stream.seekg(0);
        stream.read(buffer.data(), size);
        stream.close();
        return buffer;
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::update_uniform_buffers(uint32_t current_frame) {
        static auto startTime = chrono::high_resolution_clock::now();
        auto currentTime = chrono::high_resolution_clock::now();
        float time = chrono::duration<float, chrono::seconds::period>(currentTime - startTime).count();
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_swap_chain_extent.width / (float) m_swap_chain_extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        memcpy(m_uniform_buffers_mapped[current_frame], &ubo, sizeof(ubo));
    }   
        template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::wait_for_device_idle() {
        vkDeviceWaitIdle(m_device);
    }
    template<u32 window_width, u32 window_height>
    VkCommandBuffer Renderer<window_width, window_height>::begin_single_time_commands() {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = m_command_pool;
        alloc_info.commandBufferCount = 1;
        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);
        return command_buffer;
    }

    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::end_single_time_commands(VkCommandBuffer command_buffer) {
        vkEndCommandBuffer(command_buffer);
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        vkQueueSubmit(m_queues.graphics, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queues.graphics);
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_texture_image() {
        m_image_size = m_swap_chain_extent.height * m_swap_chain_extent.width * 4;
        m_image_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_image_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
        m_image_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            create_buffer(
                m_image_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_image_buffers[i],
                m_image_buffers_memory[i]
            );
            vkMapMemory(m_device, m_image_buffers_memory[i], 0, m_image_size, 0, &m_image_buffers_mapped[i]);
        }

        create_image(
            m_swap_chain_extent.width,
            m_swap_chain_extent.height,
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_texture_image, 
            m_texture_image_memory
        );

    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::write_image(VkCommandBuffer command_buffer, uint32_t current_frame) {
        memcpy(m_image_buffers_mapped[0], m_image_data, static_cast<size_t>(m_image_size));
        transition_image_layout(m_texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, command_buffer);
        copy_buffer_to_image(
            m_image_buffers[0], 
            m_texture_image, 
            static_cast<uint32_t>(m_swap_chain_extent.width), 
            static_cast<uint32_t>(m_swap_chain_extent.height), 
            command_buffer
        );
        transition_image_layout(m_texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, command_buffer);
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_image(
        uint32_t width, 
        uint32_t height, 
        VkFormat format, 
        VkImageTiling tiling, 
        VkImageUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        VkImage& image, 
        VkDeviceMemory& image_memory
    ) {
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = static_cast<uint32_t>(width);
        image_info.extent.height = static_cast<uint32_t>(height);
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = format;
        image_info.tiling = tiling;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = usage;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.flags = 0; // Optionals
        auto res = vkCreateImage(m_device, &image_info, nullptr, &image);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE IMAGE: {}", string_VkResult(res));
        }
        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(m_device, image, &mem_requirements);
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_types(mem_requirements.memoryTypeBits, properties);
        res = vkAllocateMemory(m_device, &alloc_info, nullptr, &image_memory);
        if (res != VK_SUCCESS) {
           panic("FAILED TO ALLOCATE TEXTURE MEMORY:", string_VkResult(res));
        }

        vkBindImageMemory(m_device, image, image_memory, 0);

    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, VkCommandBuffer command_buffer) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            panic("Unsupported layout transition");
        }

        vkCmdPipelineBarrier(
            command_buffer,
            source_stage, destination_stage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandBuffer command_buffer) {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_texture_image_view() {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_texture_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        auto res = vkCreateImageView(m_device, &view_info, nullptr, &m_texture_image_view);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE TEXTURE IMAGE VIEW: {}", string_VkResult(res));
        }
    }
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::create_texture_sampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_phy, &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        auto res = vkCreateSampler(m_device, &samplerInfo, nullptr, &m_texture_sampler);
        if (res != VK_SUCCESS) {
            panic("FAILED TO CREATE TEXTURE SAMPLER: {}", string_VkResult(res));
        }
    }
    
    template<u32 window_width, u32 window_height>
    void Renderer<window_width, window_height>::update_image(uint8_t *image_data) {
        m_image_data = image_data;
        m_image_updated = true;
    }
    template<u32 window_width, u32 window_height>
    Renderer<window_width, window_height>::~Renderer() {
        this->cleanup_swap_chain();
        vkDestroySampler(m_device, m_texture_sampler, nullptr);
        vkDestroyImageView(m_device, m_texture_image_view, nullptr);
        vkDestroyImage(m_device, m_texture_image, nullptr);
        vkFreeMemory(m_device, m_texture_image_memory, nullptr);
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        vkDestroyRenderPass(m_device, m_render_pass, nullptr);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
            vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
            vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
        }

        vkDestroyCommandPool(m_device, m_command_pool, nullptr);


        vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
        vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);

        vkDestroyBuffer(m_device, m_index_buffer, nullptr);
        vkFreeMemory(m_device, m_index_buffer_memory, nullptr);
        
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(m_device, m_uniform_buffers[i], nullptr);
            vkFreeMemory(m_device, m_uniform_buffers_memory[i], nullptr);

            vkDestroyBuffer(m_device, m_image_buffers[i], nullptr);
            vkFreeMemory(m_device, m_image_buffers_memory[i], nullptr);
        }

            
        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);

        vkDestroyDevice(m_device, nullptr);
        
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        if (enable_validation) {
            DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }
}