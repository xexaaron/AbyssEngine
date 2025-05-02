#pragma once
#include "vk/VkCommon.h"
#include "vk/VkDeviceManager.h"
#include "vk/VkShader.h"
#include "Core/Log.h"
#include <cstring>

namespace aby::vk {
	
    class Buffer {
    public:
        Buffer(std::size_t bytes, VkBufferUsageFlags flags, DeviceManager& manager);
        Buffer(const void* data, std::size_t bytes, VkBufferUsageFlags flags, DeviceManager& manager);
        virtual ~Buffer() = default;
        
        template <typename T>
        void set_data(const std::vector<T>& data, DeviceManager& manager) {
            set_data(data.data(), data.size() * sizeof(T), manager);
        }

        virtual void set_data(const void* data, std::size_t bytes, DeviceManager& manager);

        virtual void bind(VkCommandBuffer cmd) {}
        
        virtual void clear();
        
        void destroy();

        VkDeviceMemory memory();
        std::size_t size() const;

        operator VkBuffer();
    protected:
        void  create(DeviceManager& manager);
        void* map(std::size_t size, std::size_t offset = 0);
        void  unmap(void* mapped);
    protected:
        VkDevice m_Logical;
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;
        VkBufferUsageFlags m_Flags;
        std::size_t m_Size;
    };

    class VertexBuffer : public Buffer {
    public:
        // Data constructor
        template <typename T>
        VertexBuffer(const std::vector<T>& data, DeviceManager& manager) :
            Buffer(data.data(), data.size() * sizeof(T), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, manager),
            m_VertexSize(sizeof(T)),
            m_Count(data.size())
        {
        }
        VertexBuffer(const void* data, std::size_t bytes, VkDeviceSize vertex_size, DeviceManager& manager);
        VertexBuffer(std::size_t bytes, VkDeviceSize vertex_size, DeviceManager& manager);
        VertexBuffer(const VertexClass& vertex_class, DeviceManager& manager);
       
        void set_data(const void* data, std::size_t bytes, DeviceManager& manager) override;
        void clear() override;

        std::size_t count() const;
        void bind(VkCommandBuffer cmd) override;
        void print(std::ostream& os, const VertexClass& vertex_class, const ShaderDescriptor& descriptor) const;
    private:
        VkDeviceSize  m_VertexSize;
        std::size_t   m_Count;
    };

    class VertexAccumulator {
    public:
        VertexAccumulator();
        VertexAccumulator(const VertexClass& vertex_class);
        ~VertexAccumulator();

        void set_class(const VertexClass& vertex_class);
        void reset();

        std::size_t offset() const;
        std::size_t vertex_size() const;
        std::size_t capacity() const;
        std::size_t count() const;
        std::size_t bytes() const;
        void* data() const;

        VertexAccumulator& operator++();
        
        template <typename T>
        VertexAccumulator& operator=(const T& data) {
            ABY_ASSERT(sizeof(T) == m_VertexSize, "incompatible vertex size", typeid(T).name(), sizeof(T), m_VertexSize);
            ABY_ASSERT(m_Count < m_Capacity, "VertexAccumulator requires flushing!");
            std::memcpy(m_Base + offset(), &data, m_VertexSize);
            return *this;
        }

        void print(std::ostream& os, const ShaderDescriptor& descriptor) const;
    private:
        std::size_t m_Count;
        std::size_t m_Capacity;
        std::size_t m_VertexSize;
        std::byte* m_Ptr;
        std::byte* m_Base;
    };

    class IndexBuffer : public Buffer {
    public:
        IndexBuffer(const void* data, size_t size, DeviceManager& manager);
        IndexBuffer(std::size_t bytes, DeviceManager& manager);

        void bind(VkCommandBuffer cmd) override;
    };

}
