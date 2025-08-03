#pragma once
#include "Core/Common.h"
#include "Core/Log.h"
#include <unordered_map>
#include <queue>
#include <vector>
#include <any>
#include <numeric>

namespace aby {

    class Shader;
    class Texture;
    class Font;
    class Context;

    enum class EResource : u32 {
        NONE    = 0,
        SHADER,
        TEXTURE,
        FONT,
        MAX_ENUM,
    };

    template <typename T>
    concept CIsResource = std::is_same_v<T, Shader> || std::is_same_v<T, Texture> || std::is_same_v<T, Font>;

    template <typename T> requires (CIsResource<T>)
    constexpr EResource TypeToEResource() {
        if constexpr (std::is_same_v<T, Shader>) {
            return EResource::SHADER;
        }
        else if constexpr (std::is_same_v<T, Texture>) {
            return EResource::TEXTURE;
        }
        else if constexpr (std::is_same_v<T, Font>) {
            return EResource::FONT;
        } else {
            static_assert(false, "T != EResource Enumeration");
        }
    }

    class Resource {
    public:
        using Handle = u32;
        static constexpr size_t null = std::numeric_limits<Handle>::max();
    public:
        Resource();
        Resource(EResource type, Handle handle);
        Resource(const Resource&) = default;
        Resource(Resource&&) noexcept = default;

        EResource type() const;
        Handle    handle() const;

        explicit operator bool() const;
        Resource& operator=(const Resource&) = default;
        Resource& operator=(Resource&&) noexcept = default;
        bool operator==(const Resource& other) const;
        bool operator!=(const Resource& other) const;
    private:
        EResource m_Type;
        Handle m_Handle;
    };

    template <typename T> requires (CIsResource<T>)
    class IResourceHandler {
    public:
        using Handle = Resource::Handle;
    public:
        explicit IResourceHandler(std::any user_data) : m_UserData(std::move(user_data)) {}
        virtual ~IResourceHandler() = default;

        virtual void on_add(Handle handle, Ref<T> resource) = 0;
        virtual void on_erase(Handle handle, Ref<T> resource) = 0;
    protected:
        std::any m_UserData;
    };

    template <typename T> requires (CIsResource<T>)
    class ResourceClass {
    public:
        using Handle = Resource::Handle;
        
        using Handler = IResourceHandler<T>;

        template <typename Value>
        using Map = std::unordered_map<Handle, Value>;

        void assert_contains(Resource resource) const {
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
            for (auto& handler : m_Handlers) {
                handler->on_add(handle, ptr);
            }
            m_Resources.emplace(handle, std::move(ptr));
            return Resource(TypeToEResource<T>(), handle);
        }

        void add_handler(Unique<Handler>&& handler) {
            m_Handlers.push_back(std::move(handler));
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
            for (auto& handler : m_Handlers) {
                handler->on_erase(handle, m_Resources.at(handle));
            }
            m_Resources.erase(handle);
            m_RecycledHandles.push(handle);
        }

        Ref<T> at(Resource resource) {
            assert_contains(resource);
            return m_Resources.at(resource.handle());
        }

        Ref<T> at(Resource resource) const {
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
        std::vector<Unique<Handler>> m_Handlers;
    };
    


}