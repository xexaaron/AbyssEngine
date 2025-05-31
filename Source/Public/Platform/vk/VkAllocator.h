#pragma once

#include "Platform/vk/VkCommon.h"
#include "Core/Common.h"

namespace aby::vk {


    class IAllocator{
    public:
        virtual ~IAllocator() = default;
        static IAllocator& get();

        virtual void* alloc(std::size_t size, std::size_t alignment, void* user_data = nullptr) = 0;
        virtual void* realloc(void* original, std::size_t size, std::size_t alignment, void* user_data = nullptr) = 0;
        virtual void  free(void* mem, void* user_data = nullptr) = 0;
        virtual void  internal_alloc_notify(std::size_t size,void* user_data = nullptr) {}
        virtual void  internal_free_notify(std::size_t size, void* user_data = nullptr) {}

        operator VkAllocationCallbacks*();
    protected:
        IAllocator();
    protected:
        VkAllocationCallbacks m_Callbacks;
    };

    class DefaultAllocator : public IAllocator {
    public:
        virtual ~DefaultAllocator() = default;
        virtual void* alloc(std::size_t size, std::size_t alignment,void* user_data = nullptr) override;
        virtual void* realloc(void* original, std::size_t size, std::size_t alignment, void* user_data = nullptr) override;
        virtual void  free(void* mem, void* user_data = nullptr) override;
    private:
    };

    class DebugAllocator : public DefaultAllocator {
    public:
        virtual ~DebugAllocator() = default;
        virtual void* alloc(std::size_t size, std::size_t alignment, void* user_data = nullptr) override;
        virtual void* realloc(void* original, std::size_t size, std::size_t alignment, void* user_data = nullptr) override;
        virtual void  free(void* mem, void* user_data = nullptr) override;
    private:
    };

#ifdef NDEBUG
    using Allocator = DefaultAllocator;
#else
    using Allocator = DebugAllocator;
#endif

}