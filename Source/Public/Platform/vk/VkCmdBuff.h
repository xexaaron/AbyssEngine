#pragma once
#include "Platform/vk/VkCmdPool.h"
#include "Core/Common.h"

namespace aby::vk {

	class CmdBuff {
	public:
		CmdBuff();
		CmdBuff(VkDevice logical, Ref<CmdPool> pool, bool is_primary = true);

		void create(VkDevice logical, Ref<CmdPool> pool, bool is_primary = true);

		void begin();
		void end();
		
		void draw();
		void draw_indexed();

		operator VkCommandBuffer();
	private:
		Weak<CmdPool> m_Pool;
		VkCommandBuffer m_Buff;
	};

}
