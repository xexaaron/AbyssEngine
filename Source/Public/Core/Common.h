#pragma once
#include <memory>
#include <filesystem>
#include <map>
#include <vector>
#include <limits>
#include <random>
#include <type_traits>

#define abstract

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifndef NDEBUG
#define DBG(x) x 
#else 
#define DBG(x)
#endif

#if defined(_WIN32) || defined(_WIN64)
#if defined(ABY_BUILD_DLL) 
#define API(x) __declspec(dllexport)
#else
#define API(x) __declspec(dllimport)
#endif
#elif defined(__linux__) || defined(__APPLE__)
#if defined(ABY_BUILD_DLL)
#define API(x) __attribute__((visibility("default")))
#else
#define API(x)
#endif
#else
#define API(x)
#endif

#ifndef ABY_DBG_BREAK
#ifdef _MSC_VER
#define ABY_DBG_BREAK() __debugbreak()
#elif defined(__has_builtin)
#if __has_builtin(__builtin_debugtrap)
#define ABY_DBG_BREAK() __builtin_debugtrap()
#elif __has_builtin(__builtin_trap)
#define ABY_DBG_BREAK() __builtin_trap()
#endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
extern "C" int raise(int sig);
#define ABY_DBG_BREAK() raise(SIGTRAP)
#elif defined(_WIN32)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#define ABY_DBG_BREAK() DebugBreak()
#else
#define ABY_DBG_BREAK() ((void)0)
#endif
#endif

#ifndef ABY_FUNC_SIG
#ifdef __GNUC__
#define ABY_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define ABY_FUNC_SIG __FUNCSIG__
#else  // Unknown
#if (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define ABY_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define ABY_FUNC_SIG __FUNC__
#else
#define ABY_FUNC_SIG __func__
#endif // __FUNCTION__
#endif // __GNUC__
#endif

#ifndef _WIN32
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#define POSIX 
#endif
#endif

#define EXPAND_VEC2(v) v.x, v.y
#define EXPAND_VEC3(v) v.x, v.y, v.z
#define EXPAND_VEC4(v) v.x, v.y, v.z, v.w
#define EXPAND_COLOR(c) c.r, c.g, c.b, c.a

#define BIT(x) (1U << x)
#define DECLARE_ENUM_OPS(e)                             \
constexpr e operator|(e lhs, e rhs) {                   \
    return static_cast<e>(                              \
        static_cast<std::underlying_type_t<e>>(lhs) |   \
        static_cast<std::underlying_type_t<e>>(rhs)     \
    );                                                  \
}                                                       \
                                                        \
constexpr e operator&(e lhs, e rhs) {                \
    return static_cast<e>(                           \
        static_cast<std::underlying_type_t<e>>(lhs) &   \
        static_cast<std::underlying_type_t<e>>(rhs)     \
    );                                                  \
}                                                       \
                                                        \
constexpr e operator^(e lhs, e rhs) {                   \
    return static_cast<e>(                              \
        static_cast<std::underlying_type_t<e>>(lhs) ^   \
        static_cast<std::underlying_type_t<e>>(rhs)     \
    );                                                  \
}                                                       \
                                                        \
constexpr e operator~(e trait) {                        \
    return static_cast<e>(                              \
        ~static_cast<std::underlying_type_t<e>>(trait)  \
    );                                                  \
}                                                       \
constexpr e& operator|=(e& lhs, e rhs) {                \
    lhs = lhs | rhs;                                    \
    return lhs;                                         \
}                                                       \
                                                        \
constexpr e& operator&=(e& lhs, e rhs) {                \
    lhs = static_cast<e>(                               \
        static_cast<std::underlying_type_t<e>>(lhs) &   \
        static_cast<std::underlying_type_t<e>>(rhs)     \
    );                                                  \
    return lhs;                                         \
}                                                       \
                                                        \
constexpr e& operator^=(e& lhs, e rhs) {                \
    lhs = lhs ^ rhs;                                    \
    return lhs;                                         \
}                                                       


namespace fs = std::filesystem;

namespace aby {

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename Deleter = std::default_delete<T>>
    using Unique = std::unique_ptr<T, Deleter>;

    template <typename T>
    using Weak = std::weak_ptr<T>;

    template <typename T, typename... Args>
    Ref<T> create_ref(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    Unique<T> create_unique(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename Deleter, typename... Args>
    Unique<T, Deleter> create_unique_with_deleter(Deleter deleter, Args&&... args) {
        return Unique<T, Deleter>(new T(std::forward<Args>(args)...), std::move(deleter));
    }

    /**
    * @brief CreateRefEnabler class. 
    *        Used to disable easy creation of objects like Fonts through their own constructors
    *        in favor of their own static T::create(...) method. Not foolproof since anyone 
    *        can use this.
    * @tparam T Class with protected/private ctors.
    */
    template <typename T>
    struct CreateRefEnabler : public T {
    private:

        template <typename... Args>
        static Ref<T> create(Args&&... args) {
            return std::make_shared<CreateRefEnabler<T>>(std::forward<Args>(args)...);
        }
        friend T;
    public:
        template <typename... Args>
        CreateRefEnabler(Args&&... args) : T(std::forward<Args>(args)...) {}
    };
}

namespace aby {
    struct AppVersion {
        std::uint32_t major = 0;
        std::uint32_t minor = 0;
        std::uint32_t patch = 0;
    };

    enum class EBackend {
        VULKAN,
        DEFAULT = VULKAN,
    };

    struct AppInfo {
        // App name.
        std::string name     = "App";
        // App version.
        AppVersion  version  = { 0, 0, 0 };
        // Does the window inherit the name?
        bool        binherit = true; 
        // Rendering backend.
        EBackend    backend  = EBackend::DEFAULT; 
    };
    
    enum class ECursor {
        ARROW,
        IBEAM,
        CROSSHAIR,
        HAND,
        HRESIZE,
        VRESIZE,
    };

    template <typename Value, typename Fn>
    auto map_vector(const std::vector<Value>& vec, Fn&& get_key) {
        using Key = std::invoke_result_t<Fn, const Value&>;
        std::multimap<Key, Value> result;
        for (const auto& element : vec) {
            result.insert({ get_key(element), element });
        }
        return result;
    }

    template <typename T, T Seed = 0, T Min = std::numeric_limits<T>::min(), T Max = std::numeric_limits<T>::max()> requires (std::is_arithmetic_v<T>)
    class Random {
    public:
        static T gen() noexcept {
            static std::mt19937_64 generator;
            static std::once_flag flag;

            std::call_once(flag, [&]() {
                if constexpr (Seed != 0) {
                    generator.seed(Seed);
                }
                else {
                    generator.seed(std::random_device{}());
                }
                });

            if constexpr (std::is_floating_point_v<T>) {
                std::uniform_real_distribution<T> distribution(Min, Max);
                return distribution(generator);
            }
            else if constexpr (std::is_integral_v<T>) {
                std::uniform_int_distribution<T> distribution(Min, Max);
                return distribution(generator);
            }
        }

        operator T() const noexcept {
            return Random<T, Min, Max, Seed>::gen();
        }
    };

    class UUID {
    public:
        UUID() : m_Value(Random<uint64_t, 2083231>::gen()) {}
        UUID(const UUID& other) : m_Value(other.m_Value) {}
        UUID(UUID&& other) noexcept : m_Value(other.m_Value) {}

        operator std::uint64_t() const {
            return m_Value;
        }

        UUID& operator=(const UUID& other) {
            m_Value = other.m_Value;
        }

        bool operator==(const UUID& other) {
            return m_Value == other.m_Value;
        }
        bool operator!=(const UUID& other) {
            return !this->operator==(other);
        }

    private:
        uint64_t m_Value;
    };




}