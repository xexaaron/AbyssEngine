#include "vk/VkCmdPool.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"

namespace aby::vk {

	CmdPool::CmdPool(VkDevice logical, std::uint32_t queue_family_idx) {
		create(logical, queue_family_idx);
	}

	void CmdPool::create(VkDevice logical, std::uint32_t queue_family_idx) {
        VkCommandPoolCreateInfo cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family_idx
        };
        VK_CHECK(vkCreateCommandPool(logical, &cmd_pool_info, IAllocator::get(), &m_Pool));
	}

    void CmdPool::destroy(VkDevice logical) {
        vkDestroyCommandPool(logical, m_Pool, IAllocator::get());
    }

    
    CmdPool::operator VkCommandPool() {
        return m_Pool;
    }
    CmdPool::operator const VkCommandPool() const {
        return m_Pool;
    }
}