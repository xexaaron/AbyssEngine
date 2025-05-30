#include "Platform/vk/VkBuffer.h"
#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"

namespace aby::vk {

    Buffer::Buffer(std::size_t bytes, VkBufferUsageFlags flags, DeviceManager& manager) : 
        m_Logical(manager.logical()),
        m_Buffer(VK_NULL_HANDLE),
        m_Memory(VK_NULL_HANDLE),
        m_Flags(flags),
        m_Size(bytes)
    {
        create(manager);
    }

    Buffer::Buffer(const void* data, std::size_t bytes, VkBufferUsageFlags flags, DeviceManager& manager) :
        m_Logical(manager.logical()),
        m_Buffer(VK_NULL_HANDLE),
        m_Memory(VK_NULL_HANDLE),
        m_Flags(flags),
        m_Size(bytes)
    {
        create(manager);
        if (data != nullptr) {
            set_data(data, bytes, manager);
        }
    }
    
    void Buffer::create(DeviceManager& manager) {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = m_Size;
        bufferCreateInfo.usage = m_Flags;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(manager.logical(), &bufferCreateInfo, IAllocator::get(), &m_Buffer));
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(manager.logical(), m_Buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = helper::find_mem_type(
                memoryRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                manager.physical()
            )
        };

        if (allocInfo.memoryTypeIndex == UINT32_MAX) {
            throw std::runtime_error("Out of memory!");
        }

        VK_CHECK(vkAllocateMemory(manager.logical(), &allocInfo, IAllocator::get(), &m_Memory));
        VK_CHECK(vkBindBufferMemory(manager.logical(), m_Buffer, m_Memory, 0));
    }

    void Buffer::clear() {
        void* mapped = map(m_Size, 0);
        std::memset(mapped, 0, m_Size);
        unmap(mapped);
    }

    void* Buffer::map(std::size_t size, std::size_t offset) {
        void* mapped;
        VK_CHECK(vkMapMemory(m_Logical, m_Memory, offset, size, 0, &mapped));
        return mapped;
    }

    void Buffer::unmap(void* mapped) {
        vkUnmapMemory(m_Logical, m_Memory);
    }

    void Buffer::set_data(const void* data, std::size_t bytes, DeviceManager& manager) {
        if (bytes > m_Size) {
            ABY_DBG("Buffer::resize(old_size = {}, new_size = {})", m_Size, bytes);
            destroy();
            m_Size = bytes;
            create(manager);
        }
        void* mapped = map(bytes);
        std::memcpy(mapped, data, bytes);
        unmap(mapped);
    }

    void Buffer::destroy() {
        if (m_Buffer) {
            vkDestroyBuffer(m_Logical, m_Buffer, IAllocator::get());
            m_Buffer = VK_NULL_HANDLE;
        }
        if (m_Memory) {
            vkFreeMemory(m_Logical, m_Memory, IAllocator::get());
            m_Memory = VK_NULL_HANDLE;
        }
    }

    
    VkDeviceMemory Buffer::memory() {
        return m_Memory;
    }

    std::size_t Buffer::size() const {
        return m_Size;
    }

    Buffer::operator VkBuffer() {
        return m_Buffer;
    }


}

namespace aby::vk {

    VertexBuffer::VertexBuffer(const void* data, std::size_t bytes, VkDeviceSize vertex_size, DeviceManager& manager) :
        Buffer(data, bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, manager),
        m_VertexSize(vertex_size),
        m_Count(bytes / m_VertexSize)
    {
        if (m_VertexSize == 0 || bytes % m_VertexSize != 0) {
            throw std::invalid_argument("Invalid vertex size or buffer size.");
        }
    }
    // Space constructor
    VertexBuffer::VertexBuffer(std::size_t bytes, VkDeviceSize vertex_size, DeviceManager& manager) :
        Buffer(bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, manager),
        m_VertexSize(vertex_size),
        m_Count(0)
    {
        if (m_VertexSize == 0 || bytes % m_VertexSize != 0) {
            throw std::invalid_argument("Invalid vertex size or buffer size.");
        }
    }
    // Space constructor
    VertexBuffer::VertexBuffer(const VertexClass& vertex_class, DeviceManager& manager) :
        Buffer(vertex_class.max_vertices()* vertex_class.vertex_size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, manager),
        m_VertexSize(vertex_class.vertex_size()),
        m_Count(0) 
    {
    }
    
    void VertexBuffer::set_data(const void* data, std::size_t bytes, DeviceManager& manager) {
        Buffer::set_data(data, bytes, manager);
        m_Count = bytes / m_VertexSize;
    }

    std::size_t VertexBuffer::count() const {
        return m_Count;
    }

    void VertexBuffer::bind(VkCommandBuffer cmd)  {
        VkDeviceSize offset = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_Buffer, &offset);
    }

    void VertexBuffer::print(std::ostream& os, const VertexClass& vertex_class, const ShaderDescriptor& descriptor) const {
        void* mapped_memory;
        vkMapMemory(m_Logical, m_Memory, 0, m_VertexSize, 0, &mapped_memory);

        os << "{";
        size_t vertex_offset = 0;  // Tracks the current offset within the vertex buffer.

        // Loop through each vertex (assuming you know how many vertices you have, typically m_Count)
        for (std::size_t i = 0; i < m_Count; i++) {
            os << "\n  {";

            // Loop through the inputs in the ShaderDescriptor
            for (const auto& input : descriptor.inputs) {
                if (input.binding == vertex_class.binding()) {
                    // Process the data for this input
                    void* current_data = static_cast<char*>(mapped_memory) + vertex_offset;
                    os << " (";
                    switch (input.format) {
                        case VK_FORMAT_R32_SFLOAT:
                        {
                            float* data = static_cast<float*>(current_data);
                            os << *data;
                            vertex_offset += sizeof(float);
                            break;
                        }
                        case VK_FORMAT_R32G32_SFLOAT:
                        {
                            float* data = static_cast<float*>(current_data);
                            os << data[0] << ", " << data[1];
                            vertex_offset += sizeof(float) * 2;
                            break;
                        }
                        case VK_FORMAT_R32G32B32_SFLOAT:
                        {
                            float* data = static_cast<float*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2];
                            vertex_offset += sizeof(float) * 3;
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_SFLOAT:
                        {
                            float* data = static_cast<float*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3];
                            vertex_offset += sizeof(float) * 4;
                            break;
                        }
                        case VK_FORMAT_R32_SINT:
                        {
                            int* data = static_cast<int*>(current_data);
                            os << *data;
                            vertex_offset += sizeof(int);
                            break;
                        }
                        case VK_FORMAT_R32G32_SINT:
                        {
                            int* data = static_cast<int*>(current_data);
                            os << data[0] << ", " << data[1];
                            vertex_offset += sizeof(int) * 2;
                            break;
                        }
                        case VK_FORMAT_R32G32B32_SINT:
                        {
                            int* data = static_cast<int*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2];
                            vertex_offset += sizeof(int) * 3;
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_SINT:
                        {
                            int* data = static_cast<int*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3];
                            vertex_offset += sizeof(int) * 4;
                            break;
                        }
                        case VK_FORMAT_R32_UINT:
                        {
                            uint32_t* data = static_cast<uint32_t*>(current_data);
                            os << *data;
                            vertex_offset += sizeof(uint32_t);
                            break;
                        }
                        case VK_FORMAT_R32G32_UINT:
                        {
                            uint32_t* data = static_cast<uint32_t*>(current_data);
                            os << data[0] << ", " << data[1];
                            vertex_offset += sizeof(uint32_t) * 2;
                            break;
                        }
                        case VK_FORMAT_R32G32B32_UINT:
                        {
                            uint32_t* data = static_cast<uint32_t*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2];
                            vertex_offset += sizeof(uint32_t) * 3;
                            break;
                        }
                        case VK_FORMAT_R32G32B32A32_UINT:
                        {
                            uint32_t* data = static_cast<uint32_t*>(current_data);
                            os << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3];
                            vertex_offset += sizeof(uint32_t) * 4;
                            break;
                        }
                        default:
                            os << "Unsupported format";
                            break;
                    }
                    os << ") "; // Separator between components of the input.
                }
            }
            os << "}";  // End of vertex
        }
        os << "\n}\n";  // End of all vertices
        vkUnmapMemory(m_Logical, m_Memory);
    }

    void VertexBuffer::clear() {
        Buffer::clear();
        m_Count = 0;
    }


}

namespace aby::vk {

    VertexAccumulator::VertexAccumulator() :
        m_Count(0),
        m_Capacity(0),
        m_VertexSize(0),
        m_Ptr(nullptr),
        m_Base(nullptr) {
    }

    VertexAccumulator::VertexAccumulator(const VertexClass& vertex_class) :
        m_Count(0),
        m_Capacity(vertex_class.max_vertices()),
        m_VertexSize(vertex_class.vertex_size()),
        m_Ptr(new std::byte[m_Capacity * m_VertexSize]),
        m_Base(m_Ptr) {
    }

    VertexAccumulator::~VertexAccumulator() {
        delete[] m_Base;
    }

    void VertexAccumulator::set_class(const VertexClass& vertex_class) {
        this->reset();
        m_Capacity = vertex_class.max_vertices();
        m_VertexSize = vertex_class.vertex_size();
        delete[] m_Base;
        m_Ptr = new std::byte[m_Capacity * m_VertexSize];
        m_Base = m_Ptr;
    }

    void VertexAccumulator::reset() {
        m_Count = 0;
        m_Ptr = m_Base;
    }

    std::size_t VertexAccumulator::offset() const {
        return m_VertexSize * m_Count;
    }

    std::size_t VertexAccumulator::vertex_size() const {
        return m_VertexSize;
    }

    std::size_t VertexAccumulator::capacity() const {
        return m_Capacity;
    }

    std::size_t VertexAccumulator::count() const {
        return m_Count;
    }

    std::size_t VertexAccumulator::bytes() const {
        return m_VertexSize * m_Count;
    }

    void* VertexAccumulator::data() const {
        return m_Base;
    }

    VertexAccumulator& VertexAccumulator::operator++() {
        m_Count++;
        m_Ptr = m_Ptr + m_VertexSize;
        return *this;
    }
    void VertexAccumulator::print(std::ostream& os, const ShaderDescriptor& descriptor) const {
        os << "{\n";

        for (std::size_t i = 0; i < m_Count; i++) {
            auto ptr = reinterpret_cast<std::byte*>(m_Base);
            auto position = (i * m_VertexSize);
            auto begin = ptr + position;
            auto end = begin + m_VertexSize;
            auto vertex = std::span<std::byte>(begin, end);
            auto offset = 0;

            os << "  { ";

            for (const auto& input : descriptor.inputs) {
                auto data = vertex.subspan(offset, input.stride);
                offset += input.stride;
                switch (input.format) {
                case VK_FORMAT_R32_SFLOAT:
                    os << *reinterpret_cast<const float*>(data.data());
                    break;
                case VK_FORMAT_R32G32_SFLOAT:
                    os << *reinterpret_cast<const glm::vec2*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32_SFLOAT:
                    os << *reinterpret_cast<const glm::vec3*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32A32_SFLOAT:
                    os << *reinterpret_cast<const glm::vec4*>(data.data());
                    break;
                case VK_FORMAT_R32_SINT:
                    os << *reinterpret_cast<const int*>(data.data());
                    break;
                case VK_FORMAT_R32G32_SINT:
                    os << *reinterpret_cast<const glm::ivec2*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32_SINT:
                    os << *reinterpret_cast<const glm::ivec3*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32A32_SINT:
                    os << *reinterpret_cast<const glm::ivec4*>(data.data());
                    break;
                case VK_FORMAT_R32_UINT:
                    os << *reinterpret_cast<const uint32_t*>(data.data());
                    break;
                case VK_FORMAT_R32G32_UINT:
                    os << *reinterpret_cast<const glm::uvec2*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32_UINT:
                    os << *reinterpret_cast<const glm::uvec3*>(data.data());
                    break;
                case VK_FORMAT_R32G32B32A32_UINT:
                    os << *reinterpret_cast<const glm::uvec4*>(data.data());
                    break;
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
}

namespace aby::vk {

    IndexBuffer::IndexBuffer(const void* data, size_t bytes, DeviceManager& manager) :
        Buffer(data, bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, manager)
    {

    }
    IndexBuffer::IndexBuffer(std::size_t bytes, DeviceManager& manager) :
        Buffer(bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, manager)
    {

    }

    void IndexBuffer::bind(VkCommandBuffer cmd) {
        vkCmdBindIndexBuffer(cmd, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
    }

}


