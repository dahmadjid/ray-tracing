#include "renderer.hpp"
#include <fstream>
#include <vector>
#include <set>
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include <algorithm>
#include <cstring>
#include <iostream>

namespace renderer
{

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
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


    void Renderer::create_instance()
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
        bool validation_supported = true;
        for (const char *layer_name : validation_layers)
        {
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

        if (!validation_supported && enable_validation)
        {
            fmt::println("Validation layers requested, but not available!");
            exit(1);
        }
        else
        {
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
        if (enable_validation)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            create_info.pNext = &debug_create_info;
        }
        else
        {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;

        }
        auto res = vkCreateInstance(&create_info, nullptr, &m_instance);
        if (res != VK_SUCCESS)
        {
            fmt::println("FAILED TO CREATE VK INSTANCE: {}", string_VkResult(res));
            exit(1);
        }
    }

    void Renderer::create_surface()
    {
        auto res = glfwCreateWindowSurface(m_instance, m_window.m_window, nullptr, &m_surface);
        if (res != VK_SUCCESS)
        {
            fmt::println("FAILED TO CREATE VK SURFACE: {}", string_VkResult(res));
            exit(1);
        }
        fmt::println("Surface created successfully");
    }

    void Renderer::pick_physical_device()
    {
        uint32_t devices_count = 0;
        auto res = vkEnumeratePhysicalDevices(m_instance, &devices_count, NULL);
        if (res != VK_SUCCESS && res != VK_INCOMPLETE)
        {
            fmt::println("FAILED TO ENUMERATE VK PHYSICAL DEVICES: {}", string_VkResult(res));
            exit(1);
        }
        std::vector<VkPhysicalDevice> devices(devices_count);

        if (devices_count == 0)
        {
            fmt::println("NO PHYSICAL DEVICES FOUND");
            exit(1);
        }

        vkEnumeratePhysicalDevices(m_instance, &devices_count, devices.data());
        for (const auto &device : devices)
        {
            auto queues = find_queue_families(device);
            bool extension_supported = check_device_extensions_support(device);
            if (extension_supported)
            {
                auto chain_support = query_swap_chain_support(device);
                if (queues.good() && !chain_support.formats.empty() && !chain_support.present_modes.empty())
                {
                    m_phy = device;
                    m_queues_indices = queues;
                    break;
                }
            }
        }
        if (m_phy == VK_NULL_HANDLE)
        {
            fmt::println("NO SUITABLE PHYSICAL DEVICES FOUND");
            exit(1);
        }
        fmt::println("Found {} GPU", devices_count);
    }

    QueueFamilyIndices Renderer::find_queue_families(const VkPhysicalDevice &device)
    {

        uint32_t queues_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, nullptr);

        std::vector<VkQueueFamilyProperties> queues(queues_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, queues.data());
        QueueFamilyIndices queues_indices_temp;
        int i = 0;
        VkBool32 present_support = false;
        for (const auto &queue : queues)
        {
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

    bool Renderer::check_device_extensions_support(const VkPhysicalDevice &device)
    {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available_extensions.data());
        
        int required_found = 0;
        for (const auto &extension : available_extensions)
        {
            for (const auto& req: m_required_extensions) {
                if (strcmp(req, extension.extensionName) == 0) {
                    required_found += 1;
                }
            }
        }

        return m_required_extensions.size() == required_found;
    }

    void Renderer::create_logical_device()
    {
        auto queue_families = std::set({m_queues_indices.graphics.value(), m_queues_indices.presentation.value()});
        float prio = 1.0f;
        auto queues_create_infos = std::vector<VkDeviceQueueCreateInfo>();
        for (const auto &family_index : queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = family_index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &prio;
            queues_create_infos.emplace_back(queue_create_info);
        }
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.logicOp = VK_TRUE;
        VkDeviceCreateInfo create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pEnabledFeatures = &deviceFeatures;

        create_info.pQueueCreateInfos = queues_create_infos.data();
        create_info.queueCreateInfoCount = queues_create_infos.size();

        create_info.enabledExtensionCount = m_required_extensions.size();

        create_info.ppEnabledExtensionNames = m_required_extensions.data();

        auto res = vkCreateDevice(m_phy, &create_info, nullptr, &m_device);
        if (res != VK_SUCCESS)
        {
            fmt::println("FAILED TO CREATE VK LOGICAL DEVICES: {}", string_VkResult(res));
            exit(1);
        }
        vkGetDeviceQueue(m_device, m_queues_indices.graphics.value(), 0, &m_queues.graphics);
        vkGetDeviceQueue(m_device, m_queues_indices.presentation.value(), 0, &m_queues.presentation);
        fmt::println("Logical Device created successfully");
    }

    void Renderer::create_swap_chain()
    {
        auto details = query_swap_chain_support(m_phy);
        auto selected_format = details.formats[0];
        auto selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        auto actual_extent = details.capabilities.currentExtent;

        for (const auto &format : details.formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                selected_format = format;
            }
        }

        for (const auto &present_mode : details.present_modes)
        {
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
        if (selected_format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            fmt::println("Selected Image Format: VK_FORMAT_B8G8R8A8_SRGB");
        }
        else
        {
            fmt::println("Selected Image Format: {}", selected_format.format);
        }
        if (selected_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            fmt::println("Selected Presentation mode: VK_PRESENT_MODE_MAILBOX_KHR");
        }
        else
        {
            fmt::println("Selected Presentation mode: {}", selected_present_mode);
        }

        if (details.capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
        {
            int width, height;
            glfwGetFramebufferSize(m_window.m_window, &width, &height);
            actual_extent.width = std::clamp((uint32_t)width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp((uint32_t)height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
        }

        uint32_t image_count = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
            image_count = details.capabilities.maxImageCount;
        }

        fmt::println("Extent Width: {}, Extent Height: {}, Image Count: {})", actual_extent.width, actual_extent.height, image_count);

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
            fmt::println("FAILED TO CREATE SWAP CHAIN: {}", string_VkResult(res));
            exit(1);
        }

        fmt::println("Swap chain created successfully");

        vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
        m_swap_chain_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_chain_images.data());
        m_swap_chain_image_format = selected_format.format;
        m_swap_chain_extent = actual_extent;
    }

    SwapChainSupportDetails Renderer::query_swap_chain_support(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
        if (format_count != 0)
        {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
        }
        uint32_t present_modes_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, nullptr);

        if (present_modes_count != 0)
        {
            details.present_modes.resize(present_modes_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_modes_count, details.present_modes.data());
        }

        return details;
    }


    void Renderer::create_image_views() {
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
                fmt::println("FAILED TO CREATE IMAGE VIEW: {}", string_VkResult(res));
                exit(1);
            }
        }
    }

    VkShaderModule Renderer::create_shader_module(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        auto res = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO CREATE SHADER MODULE: {}", string_VkResult(res));
            exit(1);
        }
        return shaderModule;

    }

    void Renderer::create_render_pass() {
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
            fmt::println("FAILED TO CREATE RENDER PASS: {}", string_VkResult(res));
            exit(1);
        }

    }

    void Renderer::create_graphics_pipeline() {
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
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional


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
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        auto res = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipeline_layout);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO CREATE PIPELINE LAYOUT: {}", string_VkResult(res));
            exit(1);
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
            fmt::println("FAILED TO CREATE PIPELINE: {}", string_VkResult(res));
            exit(1);
        }
        
        fmt::println("Graphics pipeline created successfully");
        vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_device, frag_shader_module, nullptr);
    }

    void Renderer::create_framebuffers() {
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
                fmt::println("FAILED TO CREATE FRAMEBUFFER: {}", string_VkResult(res));
                exit(1);
            }
        }
    }

    void Renderer::create_command_pool() {

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = m_queues_indices.graphics.value();
        auto res = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO CREATE COMMAND POOL: {}", string_VkResult(res));
            exit(1);
        }
    }
    void Renderer::create_command_buffer() {

        VkCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = m_command_pool;
        cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_alloc_info.commandBufferCount = 1;

        auto res = vkAllocateCommandBuffers(m_device, &cmd_alloc_info, &m_command_buffer);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO CREATE COMMAND BUFFER: {}", string_VkResult(res));
            exit(1);
        }
    }
    void Renderer::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        auto res = vkBeginCommandBuffer(command_buffer, &beginInfo);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO BEGIN RECORDING COMMAND BUFFER: {}", string_VkResult(res));
            exit(1);
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
        vkCmdBeginRenderPass(m_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swap_chain_extent.width);
        viewport.height = static_cast<float>(m_swap_chain_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swap_chain_extent;
        vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);

        vkCmdDraw(m_command_buffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(m_command_buffer);
        res = vkEndCommandBuffer(m_command_buffer);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO RECORD COMMAND BUFFER: {}", string_VkResult(res));
            exit(1);
        }
    }

    void Renderer::create_sync_objects() {
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphore) != VK_SUCCESS ||
            vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fence) != VK_SUCCESS) 
        {
            fmt::println("FAILED TO CEATE SYNC OBJECTS");
            exit(1);
        }
    }

    Renderer::Renderer(Window &window)
        : m_window(window)
    {
        
        create_instance();
        setupDebugMessenger();
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_views();
        create_render_pass();
        create_graphics_pipeline();
        create_framebuffers();
        create_command_pool();
        create_command_buffer();
        create_sync_objects();
    }

    void Renderer::draw_frame() {
        vkWaitForFences(m_device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_in_flight_fence);

        uint32_t image_index;
        vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, m_image_available_semaphore, VK_NULL_HANDLE, &image_index);
        vkResetCommandBuffer(m_command_buffer, 0);
        record_command_buffer(m_command_buffer, image_index);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {m_image_available_semaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = waitSemaphores;
        submit_info.pWaitDstStageMask = waitStages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer;

        VkSemaphore signalSemaphores[] = {m_render_finished_semaphore};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signalSemaphores;

        auto res = vkQueueSubmit(m_queues.graphics, 1, &submit_info, m_in_flight_fence);
        if (res != VK_SUCCESS) {
            fmt::println("FAILED TO SUBMIT COMMAND BUFFER: {}", string_VkResult(res));
            exit(1);
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
        vkQueuePresentKHR(m_queues.presentation, &present_info);
    }

    void Renderer::setupDebugMessenger()
    {
        if (!enable_validation)
            return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        auto res = CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger);
        if (res != VK_SUCCESS){
            fmt::println("FAILED TO ENUMERATE VK PHYSICAL DEVICES: {}", string_VkResult(res));
            exit(1);
        }

    }

    std::vector<char> Renderer::read_shader(std::string_view file_path)
    {
        std::ifstream stream;
        stream.open(file_path.data(), std::ios_base::ate | std::ios_base::binary);
        if (!stream.good())
        {
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

    void Renderer::wait_for_device_idle() {
        vkDeviceWaitIdle(m_device);
    }

    Renderer::~Renderer()
    {
        vkDestroySemaphore(m_device, m_image_available_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_render_finished_semaphore, nullptr);
        vkDestroyFence(m_device, m_in_flight_fence, nullptr);
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        for (auto framebuffer : m_swap_chain_framebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        vkDestroyRenderPass(m_device, m_render_pass, nullptr);

        for (auto image_view : m_swap_chain_image_views) {
            vkDestroyImageView(m_device, image_view, nullptr);
        }
        vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        if (enable_validation) {
            DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }
}