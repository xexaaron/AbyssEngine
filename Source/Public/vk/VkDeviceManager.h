#pragma once
#include "vk/VkCommon.h"
#include "vk/VkInstance.h"
#include "vk/VkCmdPool.h"
#include "vk/VkCmdBuff.h"
#include "vk/VkDescriptorPool.h"
#include "Core/Common.h"
#include <xutility>

namespace aby::vk {

    struct DeviceQueue {
        std::uint32_t FamilyIdx = UINT32_MAX;
        VkQueue Queue = VK_NULL_HANDLE;
    };

    class DeviceManager {
    public:
    public:
        DeviceManager();
        DeviceManager(Instance& inst, VkSurfaceKHR surface, const std::vector<const char*>& extensions);
        ~DeviceManager();
        
        void create(Instance& instance, VkSurfaceKHR surface, const std::vector<const char*>& extensions);
        void destroy();

        Ref<CmdPool> create_cmd_pool();
        Ref<DescriptorPool> create_descriptor_pool();

        std::pair<VkPhysicalDevice, VkDevice> devices();
        VkPhysicalDevice physical();
        VkDevice logical();
        const DeviceQueue& graphics() const;

        std::uint32_t max_texture_slots() const;
    protected:
        static VkPhysicalDevice choose_best_device(VkInstance instace);
    private:
        VkPhysicalDevice m_Physical;
        VkDevice m_Logical;
        DeviceQueue m_Graphics;
        std::uint32_t m_MaxTextureSlots;
    };

}