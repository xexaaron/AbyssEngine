#include "vk/VkDeviceManager.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"
#include <cstring>
#include <vector>

namespace aby::vk {
    
    DeviceManager::DeviceManager() :
        m_Physical(VK_NULL_HANDLE),
        m_Logical(VK_NULL_HANDLE),
        m_Graphics{},
        m_MaxTextureSlots(0)
    {

    }

    DeviceManager::DeviceManager(Instance& inst, VkSurfaceKHR surface, const std::vector<const char*>& extensions) :
        m_Physical(VK_NULL_HANDLE),
        m_Logical(VK_NULL_HANDLE),
        m_Graphics{},
        m_MaxTextureSlots(0)
    {
        create(inst, surface, extensions);
    }

    DeviceManager::~DeviceManager() {
    }

    void DeviceManager::create(Instance& instance, VkSurfaceKHR surface, const std::vector<const char*>& extensions) {
        m_Physical = choose_best_device(instance);
        VK_CHECK_HANDLE(m_Physical);
        std::vector<VkQueueFamilyProperties> queue_families;
        VK_ENUMERATE(queue_families, vkGetPhysicalDeviceQueueFamilyProperties, m_Physical);

        for (uint32_t i = 0; i < queue_families.size(); i++) {
            VkBool32 supports_present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_Physical, i, surface, &supports_present);
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && supports_present) {
                m_Graphics.FamilyIdx = i;
                break;
            }
        }

        ABY_ASSERT(m_Graphics.FamilyIdx != UINT32_MAX, "Required queue family not found!");

        // Query required device features

        VkPhysicalDeviceFeatures2 query_device_features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceVulkan13Features query_vulkan13_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT query_extended_dynamic_state_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
        VkPhysicalDeviceDescriptorIndexingFeatures query_descriptor_indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
        
        query_device_features2.pNext = &query_vulkan13_features;
        query_vulkan13_features.pNext = &query_extended_dynamic_state_features;
        query_extended_dynamic_state_features.pNext = &query_descriptor_indexing_features;
        vkGetPhysicalDeviceFeatures2(m_Physical, &query_device_features2);

        ABY_ASSERT(query_vulkan13_features.dynamicRendering, "Dynamic Rendering feature is missing");
        ABY_ASSERT(query_vulkan13_features.synchronization2, "Synchronization2 feature is missing");
        ABY_ASSERT(query_extended_dynamic_state_features.extendedDynamicState, "Extended Dynamic State feature is missing");
        
        ABY_ASSERT(query_descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing, "Bindless textures feature is missing");
        ABY_ASSERT(query_descriptor_indexing_features.runtimeDescriptorArray, "Bindless textures feature is missing");
        ABY_ASSERT(query_descriptor_indexing_features.descriptorBindingPartiallyBound, "Bindless textures feature is missing");
        ABY_ASSERT(query_descriptor_indexing_features.descriptorBindingVariableDescriptorCount, "Bindless textures feature is missing");
        ABY_ASSERT(query_descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing, "Bindless textures feature is missing");
        ABY_ASSERT(query_descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind, "Bindless textures feature is missing");

        VkPhysicalDeviceDescriptorIndexingFeatures enable_indexing_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            .pNext = nullptr,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
        };
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = &enable_indexing_features,
            .extendedDynamicState = VK_TRUE
        };

        VkPhysicalDeviceVulkan13Features enable_vulkan13_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &enable_extended_dynamic_state_features,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        }; 
    
        VkPhysicalDeviceFeatures base_features{
            .samplerAnisotropy = VK_TRUE,
        };

        VkPhysicalDeviceFeatures2 enable_device_features2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &enable_vulkan13_features,
            .features = base_features
        };

        float priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = m_Graphics.FamilyIdx,
            .queueCount = 1,
            .pQueuePriorities = &priority
        };


        VkDeviceCreateInfo dci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &enable_device_features2,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_create_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = nullptr,
        };

        // Create the logical device with enabled features
        VK_CHECK(vkCreateDevice(m_Physical, &dci, IAllocator::get(), &m_Logical));
        pfn::load_functions(m_Logical);

        // Get the graphics and present queue handles
        vkGetDeviceQueue(m_Logical, m_Graphics.FamilyIdx, 0, &m_Graphics.Queue);

        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(m_Physical, &props);
        m_MaxTextureSlots = props.limits.maxPerStageDescriptorSampledImages;

        ABY_DBG("vk::DeviceManager::create");
        ABY_DBG("  Physical Device {}", props.deviceName);
        ABY_DBG("  Type            {}", helper::to_string(props.deviceType));
        ABY_DBG("  Driver Version: {}", props.driverVersion);
        ABY_DBG("  Enabled Feature(s) 10");
        ABY_DBG("  ({})   -- ({})", 1, "shaderSampledImageArrayNonUniformIndexing");
        ABY_DBG("  ({})   -- ({})", 2, "descriptorBindingUniformBufferUpdateAfterBind");
        ABY_DBG("  ({})   -- ({})", 3, "descriptorBindingSampledImageUpdateAfterBind");
        ABY_DBG("  ({})   -- ({})", 4, "descriptorBindingPartiallyBound");
        ABY_DBG("  ({})   -- ({})", 5, "descriptorBindingVariableDescriptorCount");
        ABY_DBG("  ({})   -- ({})", 6, "runtimeDescriptorArray");
        ABY_DBG("  ({})   -- ({})", 7, "extendedDynamicState");
        ABY_DBG("  ({})   -- ({})", 8, "synchronization2");
        ABY_DBG("  ({})   -- ({})", 9, "dynamicRendering");
        ABY_DBG("  ({})  -- ({})", 10, "samplerAnisotropy");
       
    }

    std::uint32_t DeviceManager::max_texture_slots() const {
        return m_MaxTextureSlots;
    }


    Ref<CmdPool> DeviceManager::create_cmd_pool() {
        return create_ref<CmdPool>(m_Logical, m_Graphics.FamilyIdx);
    }

    Ref<DescriptorPool> DeviceManager::create_descriptor_pool() {
        return create_ref<DescriptorPool>(m_Logical);
    }


    void DeviceManager::destroy() {
        vkDestroyDevice(m_Logical, IAllocator::get());
    }

    VkPhysicalDevice DeviceManager::choose_best_device(VkInstance instance) {
        std::vector<VkPhysicalDevice> devices;
        VK_ENUMERATE(devices, vkEnumeratePhysicalDevices, instance);
        VkPhysicalDevice best_device = VK_NULL_HANDLE;
        int best_score = -1;

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(device, &device_properties);

            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceFeatures(device, &device_features);

            std::vector<VkQueueFamilyProperties> queue_families;
            VK_ENUMERATE(queue_families, vkGetPhysicalDeviceQueueFamilyProperties, device);

            bool has_graphics_queue = false;
            bool has_compute_queue = false;
            for (const auto& queue_family : queue_families) {
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    has_graphics_queue = true;
                }
                if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    has_compute_queue = true;
                }
            }

            std::vector<VkExtensionProperties> extensions; 
            VK_ENUMERATE(extensions, vkEnumerateDeviceExtensionProperties, device, nullptr);
            bool has_swapchain_extension = false;
            for (const auto& extension : extensions) {
                if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    has_swapchain_extension = true;
                    break;
                }
            }

            int device_score = 0;
            if (has_graphics_queue)      device_score += 2; 
            if (has_compute_queue)       device_score += 1; 
            if (has_swapchain_extension) device_score += 3; 
            if (device_score > best_score) {
                best_score = device_score;
                best_device = device;
            }
        }
        return best_device;
    }

    VkPhysicalDevice DeviceManager::physical() {
        return m_Physical;
    }
    
    VkDevice DeviceManager::logical() {
        return m_Logical;
    }

    std::pair<VkPhysicalDevice, VkDevice> DeviceManager::devices() {
        return std::make_pair(m_Physical, m_Logical);
    }

    const DeviceQueue& DeviceManager::graphics() const {
        return m_Graphics;
    }


    
}