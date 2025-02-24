#pragma once
#include "vk/VkCommon.h"
#include "vk/VkDeviceManager.h"
#include "vk/VkShader.h"
#include "Core/Log.h"
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
        void print(std::ostream& os, const VertexClass& vertex_class, const ShaderDescriptor& descriptor);
    private:
        VkDeviceSize  m_VertexSize;
        std::uint32_t m_Count;
    };

    class VertexAccumulator {
    public:
        VertexAccumulator() :
            m_Count(0),
            m_Capacity(0),
            m_VertexSize(0),
            m_Ptr(nullptr),
            m_Base(nullptr) {}

        VertexAccumulator(const VertexClass& vertex_class) :
            m_Count(0),
            m_Capacity(vertex_class.max_vertices()),
            m_VertexSize(vertex_class.vertex_size()),
            m_Ptr(std::malloc(m_Capacity * m_VertexSize)),
            m_Base(m_Ptr) {}

        ~VertexAccumulator() {
            std::free(m_Base);
        }

        void set_class(const VertexClass& vertex_class) {
            this->reset();
            m_Count = 0;
            m_Capacity = vertex_class.max_vertices();
            m_VertexSize = vertex_class.vertex_size();
            m_Ptr = std::malloc(m_Capacity * m_VertexSize);
            m_Base = m_Ptr;
        }

        std::size_t offset() const {
            return m_VertexSize * m_Count;
        }

        std::size_t vertex_size() const {
            return m_VertexSize;
        }

        std::size_t capacity() const {
            return m_Capacity;
        }

        std::size_t count() const {
            return m_Count;
        }

        std::size_t bytes() const {
            return m_VertexSize * m_Count;
        }

        void* data() const {
            return m_Base;
        }

        void reset() {
            m_Count = 0;
            m_Ptr = m_Base;
        }

        VertexAccumulator& operator++() {
            m_Count++;
            m_Ptr = static_cast<char*>(m_Ptr) + m_VertexSize;
            return *this;
        }

        template <typename T>
        VertexAccumulator& operator=(const T& data) {
            ABY_ASSERT(sizeof(T) == m_VertexSize, "incompatible vertex size", typeid(T).name(), sizeof(T), m_VertexSize);
            ABY_ASSERT(m_Count < m_Capacity, "VertexAccumulator requires flushing!");
            std::memcpy(static_cast<char*>(m_Base) + offset(), &data, m_VertexSize);
            return *this;
        }

        void print(std::ostream& os, const ShaderDescriptor& descriptor) {
            os << "{\n";

            for (std::size_t i = 0; i < m_Count; i++) {
                auto ptr = reinterpret_cast<std::byte*>(m_Base);
                auto position = (i * m_VertexSize);
                auto begin = ptr + position;
                auto end = begin + m_VertexSize;
                auto vertex = std::span<std::byte>(begin, end);
                auto offset  = 0;

                os << "  { ";

                for (const auto& input : descriptor.inputs) {
                    auto data = vertex.subspan(offset, input.stride);
                    offset += input.stride;
                    switch (input.format) {
                        case VK_FORMAT_R32_SFLOAT:
                        { 
                            os << *reinterpret_cast<float*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32_SFLOAT:
                        { 
                            os << *reinterpret_cast<glm::vec2*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32_SFLOAT:
                        { 
                            os << *reinterpret_cast<glm::vec3*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_SFLOAT:
                        { 
                            os << reinterpret_cast<glm::vec4*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32_SINT:
                        { // Single int
                            os << *reinterpret_cast<int*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32_SINT:
                        { 
                            os << *reinterpret_cast<glm::ivec2*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32_SINT:
                        { 
                            os << *reinterpret_cast<glm::ivec3*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_SINT:
                        { 
                            os << *reinterpret_cast<glm::ivec4*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32_UINT:
                        {
                            os << *reinterpret_cast<uint32_t*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32_UINT:
                        { 
                            os << *reinterpret_cast<glm::uvec2*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32_UINT:
                        {
                            os << *reinterpret_cast<glm::uvec3*>(data.data());
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_UINT:
                        { 
                            os << *reinterpret_cast<glm::uvec4*>(data.data());
                            break;
                        }
                        default:
                            ABY_ASSERT(false, "Unsupported VkFormat");
                            break;
                    }
                    if (i != m_Count - 1) {
                        os << ", ";
                    }
                }

                os << "}\n";
            }

            os << "}\n";
        }


    protected:

    private:
        std::size_t m_Count;
        std::size_t m_Capacity;
        std::size_t m_VertexSize;
        void* m_Ptr;
        void* m_Base;
    };

    class IndexBuffer : public Buffer {
    public:
        IndexBuffer(const void* data, size_t size, DeviceManager& manager);
        IndexBuffer(std::size_t bytes, DeviceManager& manager);

        void bind(VkCommandBuffer cmd) override;
    private:
    };

}

namespace aby::vk {
    
}