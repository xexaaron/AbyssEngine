#include "Platform/vk/VkShaderModule.h"
#include "Platform/vk/VkContext.h"
#include "Platform/vk/VkTexture.h"
#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"

namespace aby::vk {

   class TextureResourceHandler : public IResourceHandler<aby::Texture> {
   public:
       TextureResourceHandler(ShaderModule* shader_module) : 
           IResourceHandler(shader_module) 
       {
           auto logical = shader_module->m_Ctx->devices().logical();
           VkDescriptorSetLayoutBinding binding[1] = {};
           binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
           binding[0].descriptorCount = 1;
           binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
           VkDescriptorSetLayoutCreateInfo info = {};
           info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
           info.bindingCount = 1;
           info.pBindings = binding;
           VK_CHECK(vkCreateDescriptorSetLayout(logical, &info, vk::Allocator::get(), &m_ImGuiLayout));
       }

       void on_add(Handle handle, Ref<aby::Texture> texture) override {
           auto  tex = std::static_pointer_cast<vk::Texture>(texture);
           tex->m_Handler      = this;
           tex->m_Handle       = handle;
           update_descriptor_sets(handle, tex);
       }
       
       void on_erase(Handle handle, Ref<aby::Texture> texture) override {

       }

       void update_descriptor_sets(Handle handle, Ref<vk::Texture> tex) {
           auto* shader_module = std::any_cast<ShaderModule*>(m_UserData);
           auto logical        = shader_module->m_Ctx->devices().logical();

           VkDescriptorImageInfo img_info{
              .sampler = tex->sampler(),
              .imageView = tex->view(),
              .imageLayout = tex->layout(),
           };

           // Write to bindless texture array.
           {
               VkWriteDescriptorSet write{
                   .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                   .pNext = nullptr,
                   .dstSet = shader_module->m_Descriptors[1],
                   .dstBinding = BINDLESS_TEXTURE_BINDING,
                   .dstArrayElement = handle,
                   .descriptorCount = 1,
                   .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   .pImageInfo = &img_info,
                   .pBufferInfo = nullptr,
                   .pTexelBufferView = nullptr,
               };
               vkUpdateDescriptorSets(logical, 1, &write, 0, nullptr);
           }
           // Write to vk::Texture::m_ImGuiID
           {
                VkDescriptorSetAllocateInfo alloc_info{
                   .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                   .pNext = nullptr,
                   .descriptorPool = shader_module->pool(),
                   .descriptorSetCount = 1,
                   .pSetLayouts = &m_ImGuiLayout,
                };
                vkAllocateDescriptorSets(logical, &alloc_info, &tex->imgui_descriptor());

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = tex->imgui_descriptor(),
                    .dstBinding = {},
                    .dstArrayElement = {},
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &img_info,
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr
               };
           
               vkUpdateDescriptorSets(logical, 1, &write, 0, nullptr);
           }
       }
   private:
       VkDescriptorSetLayout m_ImGuiLayout;
   };

}

namespace aby::vk {

    ShaderModule::ShaderModule(vk::Context* ctx, const fs::path& vertex, const fs::path& frag) :
        m_Ctx(ctx),
        m_Layout(VK_NULL_HANDLE),
        m_Vertex(aby::Shader::create(ctx, vertex, EShader::VERTEX)),
        m_Fragment(aby::Shader::create(ctx, frag, EShader::FRAGMENT)),
        m_Pool(VK_NULL_HANDLE),
        m_Descriptors(),
        m_Uniforms(VK_NULL_HANDLE),
        m_UniformMemory(VK_NULL_HANDLE),
        m_Class(std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex))->descriptor(), 10000, 0)
    {
        m_Ctx->textures().add_handler(create_unique<TextureResourceHandler>(this));
        auto vert_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex));
        auto frag_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Fragment));

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
           vert_shader->layout(),
           frag_shader->layout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
            .pSetLayouts = descriptor_set_layouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };

        std::vector<VkDescriptorPoolSize> pool_sizes = {
           { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1 }, // Adjust counts based on needs
           { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_BINDLESS_RESOURCES }
        };

        VkDescriptorPoolCreateInfo ci{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 20,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
        };

        auto logical = m_Ctx->devices().logical();

        VK_CHECK(vkCreateDescriptorPool(logical, &ci, IAllocator::get(), &m_Pool));
        
        auto descriptor_set_count = static_cast<u32>(descriptor_set_layouts.size());

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT alloc_count_info{};
        auto max_binding = MAX_BINDLESS_RESOURCES - 1;
        std::vector<u32> max_bindings(descriptor_set_count, max_binding);
        alloc_count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        alloc_count_info.pNext = nullptr;
        alloc_count_info.descriptorSetCount = static_cast<u32>(max_bindings.size());
        alloc_count_info.pDescriptorCounts  = max_bindings.data();


        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = &alloc_count_info;
        allocInfo.descriptorPool = m_Pool;
        allocInfo.descriptorSetCount = descriptor_set_count;
        allocInfo.pSetLayouts = descriptor_set_layouts.data();

        m_Descriptors.resize(descriptor_set_count, VK_NULL_HANDLE);
        VK_CHECK(vkAllocateDescriptorSets(logical, &allocInfo, m_Descriptors.data()));
        
        VK_CHECK(vkCreatePipelineLayout(logical, &pipelineLayoutInfo, IAllocator::get(), &m_Layout));
    
        if (vert_shader->descriptor().uniforms.empty()) {
            create_uniform_buffer(0);
            update_descriptor_set(0, 0);
        }
    }


    Ref<ShaderModule> ShaderModule::create(vk::Context* ctx, const fs::path& vert, const fs::path& frag) {
        return create_ref<ShaderModule>(ctx, vert, frag);
    }

    void ShaderModule::create_uniform_buffer(std::size_t size) {
        if (size == 0) {
            size = 16;
        }

        auto logical = m_Ctx->devices().logical();
        auto physical = m_Ctx->devices().physical();

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(logical, &buffer_info, IAllocator::get(), &m_Uniforms));

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(logical, m_Uniforms, &mem_requirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = helper::find_mem_type(mem_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physical);

        VK_CHECK(vkAllocateMemory(logical, &alloc_info, IAllocator::get(), &m_UniformMemory));
        VK_CHECK(vkBindBufferMemory(logical, m_Uniforms, m_UniformMemory, 0));
    }

    void ShaderModule::destroy() {
        auto logical = m_Ctx->devices().logical();

        if (m_Uniforms != VK_NULL_HANDLE) {
            vkDestroyBuffer(logical, m_Uniforms, IAllocator::get());
            m_Uniforms = VK_NULL_HANDLE;
        }

        if (m_UniformMemory != VK_NULL_HANDLE) {
            vkFreeMemory(logical, m_UniformMemory, IAllocator::get());
            m_UniformMemory = VK_NULL_HANDLE;
        }
        if (m_Layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(logical, m_Layout, IAllocator::get());
        }

        if (!m_Descriptors.empty()) {
            for (std::size_t i = 0; i < m_Descriptors.size(); i++) {
                vkFreeDescriptorSets(logical, m_Pool, 1, &m_Descriptors[i]);
            }
            m_Descriptors.clear();
        }
        if (m_Pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(logical, m_Pool, IAllocator::get());
        }

        auto vert_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex));
        auto frag_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Fragment));
        vert_shader->destroy();
        frag_shader->destroy();
    }

    void ShaderModule::set_uniforms(const void* data, std::size_t bytes, u32 binding) {
        std::size_t expected_size = 0;
        auto vert_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex));
        for (auto& uniform : vert_shader->descriptor().uniforms) {

            expected_size += uniform.size;
        }
        ABY_ASSERT(bytes == expected_size, "Expected size {}, but got {}", expected_size, bytes);

        if (m_Uniforms == VK_NULL_HANDLE || m_UniformMemory == VK_NULL_HANDLE) {
            create_uniform_buffer(bytes);
        }
        
        update_uniform_memory(data, bytes);
        update_descriptor_set(binding, bytes);
    }

    void ShaderModule::update_uniform_memory(const void* data, std::size_t bytes) {
        auto logical = m_Ctx->devices().logical();
        void* mapped;
        vkMapMemory(logical, m_UniformMemory, 0, bytes, 0, &mapped);
        memcpy(mapped, data, bytes);
        vkUnmapMemory(logical, m_UniformMemory);
    }

    void ShaderModule::update_descriptor_set(u32 binding, std::size_t bytes) {
        auto logical = m_Ctx->devices().logical();
        std::vector<VkWriteDescriptorSet> writes;
        if (bytes == 0) {
            return;
        }
        VkDescriptorBufferInfo buffer_info{
            .buffer = m_Uniforms,
            .offset = 0,
            .range = bytes,
        };
        VkWriteDescriptorSet write_uniforms{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_Descriptors[0],
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info,
            .pTexelBufferView = nullptr
        };
        writes.push_back(write_uniforms);
        vkUpdateDescriptorSets(logical, static_cast<u32>(writes.size()), writes.data(), 0, nullptr);
    }

    Resource ShaderModule::vert() const {
        return m_Vertex;
    }

    Resource ShaderModule::frag() const {
        return m_Fragment;
    }
        
    VkPipelineLayout ShaderModule::layout() const {
        return m_Layout;
    }

    const std::vector<VkDescriptorSet>& ShaderModule::descriptors() const {
        return m_Descriptors;
    }

    std::vector<VkDescriptorSet>& ShaderModule::descriptors() {
        return m_Descriptors;
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderModule::stages() const {
        auto vert_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex));
        auto frag_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Fragment));
        return { vert_shader->stage(), frag_shader->stage() };
    }
    
    const VertexClass& ShaderModule::vertex_class() const {
        return m_Class;
    }
    
    const ShaderDescriptor& ShaderModule::vertex_descriptor() const {
        auto vert_shader = std::static_pointer_cast<vk::Shader>(m_Ctx->shaders().at(m_Vertex));
        return vert_shader->descriptor();
    }
    
    VkDescriptorPool ShaderModule::pool() {
        return m_Pool;
    }

}