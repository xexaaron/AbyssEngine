#pragma once

#include "Platform/vk/VkCommon.h"

namespace aby::vk {

    class DescriptorPool {
    public:
        DescriptorPool();
        DescriptorPool(VkDevice logical);
    
        void create(VkDevice logical);

        VkDescriptorSet alloc(VkDescriptorSetLayout layout);
        VkDescriptorSet alloc(const std::vector<VkDescriptorSetLayout>& layouts);

        operator VkDescriptorPool(); 
    private:
        VkDevice m_Logical;
        VkDescriptorPool m_Pool;
    };

}