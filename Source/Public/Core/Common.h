#pragma once
#include <memory>
#include <filesystem>
#include <map>
#include <vector>
#include <limits>
#include <type_traits>
#include <mutex>
#include <utility>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#undef min
#undef max

#define MACRO_SCOPE(x) do { x; } while(0)

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

#ifndef _WIN32
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#define POSIX 
#endif
#endif

#ifndef ABY_DBG_BREAK
#ifdef _MSC_VER
#define ABY_DBG_BREAK() __debugbreak(); 
#elif defined(__has_builtin)
#if __has_builtin(__builtin_debugtrap)
#define ABY_DBG_BREAK() __builtin_debugtrap()
#elif __has_builtin(__builtin_trap)
#define ABY_DBG_BREAK() __builtin_trap()
#endif
#elif defined(POSIX)
extern "C" int raise(int sig);
#define ABY_DBG_BREAK() raise(SIGTRAP)
#elif defined(_WIN32)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#define ABY_DBG_BREAK() DebugBreak()
#else
#define ABY_DBG_BREAK() std::system("pause"); std::abort(-1);
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

#ifdef _MSC_VER
    #define OVERRIDEABLE __declspec(selectany)
#elif defined(__GNUC__) || defined(__clang__)
    #define OVERRIDEABLE __attribute__((weak))
#else
    #pragma message("Extern function defintions are required")
    #define OVERRIDEABLE
#endif

#ifndef NDEBUG
#define IF_DBG(x, el) x 
#else 
#define IF_DBG(x, el) el
#endif

#define abstract
#define ABY_LOG(...) aby::Logger::log(__VA_ARGS__)
#define ABY_ERR(...) aby::Logger::error(__VA_ARGS__)
#define ABY_WARN(...) aby::Logger::warn(__VA_ARGS__)
#define ABY_DBG(...) IF_DBG(aby::Logger::debug(__VA_ARGS__), )
#define ABY_ASSERT(condition, ...)                                                     \
    IF_DBG(                                                                            \
        do {                                                                           \
            if (!(condition)) {                                                        \
                aby::Logger::Assert("File:{}:{}\n{}",                                  \
                    std::string_view(__FILE__).substr(                                 \
                        std::string_view(__FILE__).find_last_of("/\\") + 1),           \
                    __LINE__,                                                          \
                    ABY_FUNC_SIG                                                       \
                );                                                                     \
                aby::Logger::Assert("({})\n" __VA_OPT__(": {}"), #condition __VA_OPT__(, std::format(__VA_ARGS__))); \
                aby::Logger::flush();                                                  \
                ABY_DBG_BREAK();                                                       \
            }                                                                          \
        } while (0),                                                                   \
    condition;)

#define EXPAND_VEC2(v) v.x, v.y
#define EXPAND_VEC3(v) v.x, v.y, v.z
#define EXPAND_VEC4(v) v.x, v.y, v.z, v.w
#define EXPAND_COLOR(c) c.r, c.g, c.b, c.a
#define BIT(x) (1U << x)
#define CONCAT(x, y) x##y
#define DECLARE_ENUM_OP_EQ(e, op) constexpr e& operator op##=(e& lhs, e rhs) { lhs = static_cast<e>(std::to_underlying(lhs) op std::to_underlying(rhs)); return lhs; }
#define DECLARE_ENUM_OP(e, op) constexpr e operator op(e lhs, e rhs) { return static_cast<e>(std::to_underlying(lhs) op std::to_underlying(rhs)); }

#define DECLARE_ENUM_OPS(e) \
    DECLARE_ENUM_OP(e, |)   \
    DECLARE_ENUM_OP(e, &)   \
    DECLARE_ENUM_OP(e, ^)   \
    DECLARE_ENUM_OP_EQ(e, |) \
    DECLARE_ENUM_OP_EQ(e, &) \
    constexpr e operator~(e trait) { return static_cast<e>(~std::to_underlying(trait)); }

namespace fs = std::filesystem;

namespace aby {

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

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
        explicit CreateRefEnabler(Args&&... args) : T(std::forward<Args>(args)...) {}
    };

}

namespace aby {
    struct AppVersion {
       u32 major = 0;
       u32 minor = 0;
       u32 patch = 0;
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

}