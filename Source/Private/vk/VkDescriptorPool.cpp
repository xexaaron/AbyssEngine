#include "vk/VkDescriptorPool.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"
namespace aby::vk {
    DescriptorPool::DescriptorPool() :
        m_Logical(VK_NULL_HANDLE),
        m_Pool(VK_NULL_HANDLE)
    {

    }

    DescriptorPool::DescriptorPool(VkDevice logical) :
        m_Pool(VK_NULL_HANDLE),
        m_Logical(VK_NULL_HANDLE)
    {
        create(logical);
    }

    void DescriptorPool::create(VkDevice logical) {
        m_Logical = logical;

        std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2 }, // Adjust counts based on needs
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         5 }
        };
        VkDescriptorPoolCreateInfo ci{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext   = nullptr,
            .flags   = 0,
            .maxSets = 20,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
        };
        VK_CHECK(vkCreateDescriptorPool(logical, &ci, IAllocator::get(), &m_Pool));
    }

    VkDescriptorSet DescriptorPool::alloc(VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_Pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;
        VK_CHECK(vkAllocateDescriptorSets(m_Logical, &allocInfo, &descriptorSet));
        return std::move(descriptorSet);
    }

    VkDescriptorSet DescriptorPool::alloc(const std::vector<VkDescriptorSetLayout>& layouts) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_Pool;
        allocInfo.descriptorSetCount = static_cast<std::uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();
       
        VkDescriptorSet descriptorSet;
        VK_CHECK(vkAllocateDescriptorSets(m_Logical, &allocInfo, &descriptorSet));
        return descriptorSet;
    }


    DescriptorPool::operator VkDescriptorPool() {
        return m_Pool;
    }

}