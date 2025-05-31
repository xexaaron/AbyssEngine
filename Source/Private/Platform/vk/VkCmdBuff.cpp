#include "Platform/vk/VkCmdBuff.h"
#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"
namespace aby::vk {

	CmdBuff::CmdBuff() :
		m_Pool(),
		m_Buff(VK_NULL_HANDLE)
	{

	}

	CmdBuff::CmdBuff(VkDevice logical, Ref<CmdPool> pool, bool is_primary) : m_Pool(pool) {
		create(logical, pool, is_primary);
	}

	void CmdBuff::create(VkDevice logical, Ref<CmdPool> pool, bool is_primary) {
		ABY_ASSERT(pool, "Invalid usage of CmdBuff::create.");
		m_Pool = pool;
		const VkCommandBufferAllocateInfo cmd = {
		  .sType	   = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		  .pNext	   = nullptr,
		  .commandPool = static_cast<VkCommandPool>(*pool),
		  .level	   = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
		  .commandBufferCount = 1,
		};
		VK_CHECK(vkAllocateCommandBuffers(logical, &cmd, &m_Buff));
	}



	void CmdBuff::begin() {
		VkCommandBufferBeginInfo bi = {};
		bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 
		bi.pInheritanceInfo = nullptr;  

		VK_CHECK(vkBeginCommandBuffer(m_Buff, &bi));
	}

	void CmdBuff::end() {
		VK_CHECK(vkEndCommandBuffer(m_Buff));
	}

	CmdBuff::operator VkCommandBuffer() {
		return m_Buff;
	}

	void CmdBuff::draw() {
	}


	void CmdBuff::draw_indexed() {
	}

}