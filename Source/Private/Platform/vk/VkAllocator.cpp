#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"
#include <cstdlib>

#ifdef _MSC_VER
    #include <crtdbg.h>  // MSVC CRT Debug functions
#endif

#ifdef _MSC_VER
    #define MSVC(x) x
#else
    #define MSVC(x)
#endif

#define MSVC_CHECK(x, ...) MSVC(ABY_ASSERT(x, __VA_ARGS__))

namespace aby::vk {

    IAllocator::IAllocator() {
        m_Callbacks.pUserData = this;
        m_Callbacks.pfnAllocation = [](void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void* {
            return static_cast<IAllocator*>(user_data)->alloc(size, alignment, user_data);
        };
        m_Callbacks.pfnReallocation = [](void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void* {
            return static_cast<IAllocator*>(user_data)->realloc(original, size, alignment, user_data);
        };
        m_Callbacks.pfnFree = [](void* user_data, void* mem) {
            static_cast<IAllocator*>(user_data)->free(mem, user_data);
        };
        m_Callbacks.pfnInternalAllocation = [](void* user_data, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {
            static_cast<IAllocator*>(user_data)->internal_alloc_notify(size, user_data);
        };
        m_Callbacks.pfnInternalFree = [](void* user_data, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {
            static_cast<IAllocator*>(user_data)->internal_free_notify(size, user_data);
        };
    }

    IAllocator& IAllocator::get() {
        static Allocator allocator;
        return allocator;
    }

    IAllocator::operator VkAllocationCallbacks*() {
        return &m_Callbacks;
    }



}

namespace aby::vk {

    void* DefaultAllocator::alloc(std::size_t size, std::size_t alignment, void* user_data) {
        return std::malloc(size);
    }

    void* DefaultAllocator::realloc(void* original, std::size_t size, std::size_t alignment, void* user_data) {
        return std::realloc(original, size);
    }
    
    void DefaultAllocator::free(void* mem, void* user_data) {
        std::free(mem);
    }

}

namespace aby::vk {

    void* DebugAllocator::alloc(std::size_t size, std::size_t alignment, void* user_data) {
        MSVC(_CrtCheckMemory());
        void* p = DefaultAllocator::alloc(size, alignment, user_data);
        MSVC_CHECK(_CrtCheckMemory(), "[IAllocator::alloc] Heap corruption detected.");
        return p;
    }

    void* DebugAllocator::realloc(void* original, std::size_t size, std::size_t alignment, void* user_data) {
        MSVC(_CrtCheckMemory());
        void* p = DefaultAllocator::realloc(original, size, alignment);
        MSVC_CHECK(_CrtCheckMemory(), "[IAllocator::realloc] Heap corruption detected.");
        return p;
    }

    void DebugAllocator::free(void* mem, void* user_data) {
        MSVC(_CrtCheckMemory());
        if (mem == nullptr) { return; }
        DefaultAllocator::free(mem, user_data);
        MSVC_CHECK(_CrtCheckMemory(), "[IAllocator::free] Heap corruption detected.");
    }

}