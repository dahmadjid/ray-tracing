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

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
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
        m_swap_chain_image_views.resize(m_swap_chain_image_views.size());
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

    void Renderer::create_graphics_pipeline() {
        
    }
    Renderer::Renderer(Window &window, std::string_view vertex_shader_path, std::string_view fragment_shader_path)
        : m_window(window)
    {
        
        create_instance();
        setupDebugMessenger();
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_views();
        create_graphics_pipeline();
        
        auto vertex_shader = read_shader(vertex_shader_path);
        auto fragment_shader = read_shader(fragment_shader_path);
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

    Renderer::~Renderer()
    {
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