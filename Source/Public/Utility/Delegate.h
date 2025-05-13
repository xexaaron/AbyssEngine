#pragma once

#include <gch/small_vector.hpp>
#include "Core/Common.h"
#include "Core/Log.h"
#include <functional>
#include <type_traits>

namespace aby::util {

    enum class EDelegate {
        SINGLE,
        MULTI,
    };

    template <EDelegate type, typename Fn>
    class IDelegate {
    public:
        void bind(Fn&& fn) {
            if constexpr (type == EDelegate::SINGLE) {
                if (m_Storage.size() == 1) {
                    m_Storage[0] = std::forward<Fn>(fn);
                    return;
                }
            }
            m_Storage.push_back(std::forward<Fn>(fn));
        }

        void bind(const Fn& fn) {
            if constexpr (type == EDelegate::SINGLE) {
                if (m_Storage.size() == 1) {
                    m_Storage[0] = std::forward<Fn>(fn);
                    return;
                }
            }
            m_Storage.push_back(std::forward<Fn>(fn));
        }

    protected:
        gch::small_vector<Fn, 1> m_Storage;
    };

    template <typename T, typename Enable = void>
    class Delegate;

    template <typename R, typename... Args>
    class Delegate<R(*)(Args...)> : public IDelegate<EDelegate::SINGLE, R(*)(Args...)> {
    public:
        IDelegate<EDelegate::SINGLE, R(*)(Args...)>& base() {
            return *this;
        }

        R call(Args&&... args) {
            return IDelegate<EDelegate::SINGLE, R(*)(Args...)>::m_Storage[0](std::forward<Args>(args)...);
        }
    };

    template <typename T, typename R, typename... Args>
    class Delegate<R(T::*)(Args...)> : public IDelegate<EDelegate::SINGLE, R(T::*)(Args...)> {
    public:
        IDelegate<EDelegate::SINGLE, R(T::*)(Args...)>& base() {
            return *this;
        }

        void bind(T* obj, R(T::*fn)(Args...)) {
            if (IDelegate<EDelegate::SINGLE, R(T::*)(Args...)>::m_Storage.size() == 1) {
                IDelegate<EDelegate::SINGLE, R(T::*)(Args...)>::m_Storage[0] = [fn = std::forward<decltype(fn)>(fn)](Args&&... args) {
                    return (obj->*fn)(args...);
                };
            }
            IDelegate<EDelegate::SINGLE, R(T::*)(Args...)>::m_Storage.push_back(std::forward<decltype(fn)>(fn));
        }
   
        R call(Args&&... args) {
            return IDelegate<EDelegate::SINGLE, R(T::*)(Args...)>::m_Storage[0](std::forward<Args>(args)...);
        }
    };
    
    template <typename Lambda> requires (std::is_class_v<Lambda>)
    class Delegate<Lambda> : public IDelegate<EDelegate::SINGLE, Lambda> {
    public:
        IDelegate<EDelegate::SINGLE, Lambda>& base() {
            return *this;
        }

        auto call(auto&&... args) {
            return IDelegate<EDelegate::SINGLE, Lambda>::m_Storage[0](std::forward<decltype(args)>(args)...);
        }
    };
    
    template <typename Fn>
    class IMulticastDelegate : public IDelegate<EDelegate::MULTI, Fn> {
    public:
        std::size_t bound() const {
            return IDelegate<EDelegate::MULTI, Fn>::m_Storage.size();
        }
        auto& base() {
            return static_cast<IDelegate<EDelegate::MULTI, Fn>&>(*this);
        }
    private:
    };

    template <typename T, typename Enable = void>
    class MulticastDelegate;

    template <typename... Args>
    class MulticastDelegate<void(*)(Args...)> : public IMulticastDelegate<void(*)(Args...)> {
    public:
        void call(Args&&... args) {
            for (auto& fn : IMulticastDelegate<void(*)(Args...)>::m_Storage) {
                fn(std::forward<Args>(args)...);
            }
        }
    };

    template <typename T, typename... Args>
    class MulticastDelegate<void(T::*)(Args...)> : public IMulticastDelegate<void(T::*)(Args...)> {
    public:
        void bind(T* obj, void(T::* fn)(Args...)) {
            IMulticastDelegate<void(T::*)(Args...)>::m_Storage.push_back([fn = std::forward<decltype(fn)>(fn)](Args&&... args) {
                return (obj->*fn)(args...);
            });
        }

        void call(Args&&... args) {
            for (auto& fn : IMulticastDelegate<void(T::*)(Args...)>::m_Storage) {
                fn(std::forward<Args>(args)...);
            }
        }
    };

    template <typename Lambda> requires (std::is_class_v<Lambda>)
    class MulticastDelegate<Lambda> : public IMulticastDelegate<Lambda> {
    public:
        void call(auto&&... args) {
            for (auto& fn : IDelegate<EDelegate::MULTI, Lambda>::m_Storage) {
                fn(std::forward<decltype(args)>(args)...);
            }
        }
    };

}
