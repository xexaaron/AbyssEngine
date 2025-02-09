#pragma once
#include "Core/Common.h"
#include "Core/Log.h"
#include <unordered_map>
#include <queue>

namespace aby {

    class Shader;
    class Texture;
    class Context;

    enum class EResource : std::uint32_t {
        NONE    = 0,
        SHADER,
        TEXTURE
    };

    template <typename T>
    concept CIsResource = std::is_same_v<T, Shader> || std::is_same_v<T, Texture>;

    template <typename T> requires (CIsResource<T>)
    constexpr EResource TypeToEResource() {
        if constexpr (std::is_same_v<T, Shader>) {
            return EResource::SHADER;
        }
        else if constexpr (std::is_same_v<T, Texture>) {
            return EResource::TEXTURE;
        }
        return EResource::NONE;
    }

    class Resource {
    public:
        using Handle = std::uint32_t;
    public:
        Resource() : m_Type(EResource::NONE), m_Handle(0) {}
        Resource(EResource type, Handle handle) : m_Type(type), m_Handle(handle) {}
        Resource(const Resource& other) : m_Type(other.m_Type), m_Handle(other.m_Handle) {}
        Resource(Resource&& other) noexcept : m_Type(other.m_Type), m_Handle(other.m_Handle) {}

        EResource type() const { return m_Type; }
        Handle    handle() const { return m_Handle; }

        explicit operator bool() const {
            return (m_Type != EResource::NONE);
        }
        Resource& operator=(const Resource& other) {
            m_Type = other.m_Type;
            m_Handle = other.m_Handle;
            return *this;
        }
        Resource& operator=(Resource&& other) noexcept {
            m_Type = other.m_Type;
            m_Handle = other.m_Handle;
            return *this;
        }
        bool operator==(const Resource& other) {
            return ((m_Type == other.m_Type) && (m_Handle == other.m_Handle));
        }
        bool operator!=(const Resource& other) {
            return !this->operator==(other);
        }
    private:
        EResource m_Type;
        Handle m_Handle;
    };

    template <typename T> requires (CIsResource<T>)
    class ResourceClass {
    public:
        using Handle = typename Resource::Handle;
    
        template <typename Value>
        using Map = std::unordered_map<Handle, Value>;

        inline void assert_contains(Resource resource) const {
            ABY_ASSERT(resource.type() == TypeToEResource<T>(), "Resource type mismatch");
            ABY_ASSERT(m_Resources.contains(resource.handle()), "Resource(Type: {}, Handle: {}) not found!",
                static_cast<std::underlying_type_t<EResource>>(resource.type()),
                resource.handle()
            );
        }
    public:
        ResourceClass() : m_NextHandle(0) {}

        Resource add(Ref<T> ptr) {
            Handle handle = get_next_handle();
            m_Resources.emplace(handle, std::move(ptr));
            return Resource(TypeToEResource<T>(), handle);
        }

        template <typename... Args> requires (std::is_constructible_v<T, Args...>)
        Resource emplace(Args&&... args) {
            Handle handle = get_next_handle();
            m_Resources.emplace(handle, std::make_shared<T>(std::forward<Args>(args)...));
            return Resource(TypeToEResource<T>(), handle);
        }

        void erase(Resource resource) {
            assert_contains(resource);
            auto handle = resource.handle();
            m_Resources.erase(handle);
            m_RecycledHandles.push(handle);
        }

        Ref<T> at(Resource resource) {
            assert_contains(resource);
            return m_Resources.at(resource.handle());
        }

        const Ref<T> at(Resource resource) const {
            assert_contains(resource);
            return m_Resources.at(resource.handle());
        }

        std::size_t size() const {
            return m_Resources.size();
        }

        void clear() {
            m_Resources.clear();
        }

        auto begin() {
            return m_Resources.begin();
        }
        auto end() {
            return m_Resources.end();
        }
        auto begin() const {
            return m_Resources.begin();
        }
        auto end() const {
            return m_Resources.end();
        }
    private:
        Handle get_next_handle() {
            if (!m_RecycledHandles.empty()) {
                Handle handle = m_RecycledHandles.front();
                m_RecycledHandles.pop();
                return handle;
            }
            return m_NextHandle++;
        }
    private:
        Handle m_NextHandle;
        Map<Ref<T>> m_Resources;
        std::queue<Handle> m_RecycledHandles;
    };

}