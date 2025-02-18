#pragma once

#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <tuple>
#include <source_location>
#include <filesystem>
#include <glm/glm.hpp>
#include "Core/Common.h"

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

#ifndef ABY_LOG
#define ABY_LOG(...) aby::Logger::Log(__VA_ARGS__)
#endif
#ifndef ABY_ERR
#define ABY_ERR(...) aby::Logger::Error(__VA_ARGS__)
#endif
#ifndef ABY_WARN
#define ABY_WARN(...) aby::Logger::Warn(__VA_ARGS__)
#endif
#ifndef NDEBUG
    #ifndef ABY_DBG
        #define ABY_DBG(...) aby::Logger::Debug(__VA_ARGS__)
    #endif
  #ifndef ABY_ASSERT
    #define ABY_ASSERT(condition, ...)                                                  \
        do {                                                                            \
            if (!(condition)) {                                                         \
                aby::Logger::Assert("{}:{} {}",                                         \
                    std::string_view(__FILE__).substr(                                  \
                        std::string_view(__FILE__).find_last_of("/\\") + 1),            \
                    __LINE__,                                                           \
                    ABY_FUNC_SIG                                                        \
                );                                                                      \
                aby::Logger::Assert("!({})\n{}", #condition, std::format(__VA_ARGS__)); \
                ABY_DBG_BREAK();                                                        \
            }                                                                           \
        } while (0)
#endif
#else
    #ifndef ABY_DBG
        #define ABY_DBG(...)
    #endif

    #ifndef ABY_ASSERT
        #define ABY_ASSERT(...)
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

namespace aby {

    class Logger {
    private:
        enum class EColor {
            Black = 30, Red = 31, Green = 32, Yellow = 33, Blue = 34,
            Magenta = 35, Cyan = 36, White = 37, Reset = 0,
            
            BrightBlack = 90, BrightRed = 91, BrightGreen = 92, BrightYellow = 93,
            BrightBlue = 94, BrightMagenta = 95, BrightCyan = 96, BrightWhite = 97,
            
            Grey1 = 232, Grey2 = 233, Grey3 = 234, Grey4 = 235, Grey5 = 236,
            Grey6 = 237, Grey7 = 238, Grey8 = 239, Grey9 = 240, Grey10 = 241,
            Grey11 = 242, Grey12 = 243, Grey13 = 244, Grey14 = 245, Grey15 = 246,
            Grey16 = 247, Grey17 = 248, Grey18 = 249, Grey19 = 250, Grey20 = 251,
            Grey21 = 252, Grey22 = 253, Grey23 = 254, Grey24 = 255
        };
        template <typename... Args>
        inline static void Print(const std::string& context, EColor color, std::ostream& os, std::format_string<Args...> fmt, Args&&... args) {
            std::string prefix = std::format("[{}]", context);
            std::string msg    = std::format(fmt, std::forward<Args>(args)...);
            os << "\033[" << static_cast<int>(color) << "m" << prefix << " " << msg << "\033[0m" << '\n';
        }
    public:
        static void SetStreams(std::ostream& log_stream = std::cout, std::ostream& err_stream = std::cerr) {
            m_LogStream = &log_stream;
            m_ErrStream = &err_stream;
        }

        template <typename... Args>
        static void Log(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            Print("Info", EColor::Grey4, *m_LogStream, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void Warn(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            Print("Warn", EColor::Yellow, *m_LogStream, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void Error(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            Print("Error", EColor::BrightRed, *m_ErrStream, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void Assert(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            Print("Assert", EColor::BrightRed, *m_ErrStream, fmt, std::forward<Args>(args)...);
        }
        
        template <typename... Args>
        static void Debug(std::format_string<Args...> fmt, Args&&... args) {
        #ifndef NDEBUG
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            Print("Debug", EColor::Cyan, *m_LogStream, fmt, std::forward<Args>(args)...);
        #endif
        }

    private:

    private:
        static inline std::ostream* m_LogStream = &std::cout;
        static inline std::ostream* m_ErrStream = &std::cerr;
        static inline std::mutex    m_StreamMutex;
    };

} 

namespace std {
    template <>
    struct formatter<source_location> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }
        
        template <typename FmtContext>
        FmtContext::iterator format(const source_location& loc, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}:{} {}", loc.file_name(), loc.line(), loc.function_name());
        }
    };

    template <>
    struct formatter<filesystem::path> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        FmtContext::iterator format(const filesystem::path& path, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}", path.string());
        }
    };

    template <>
    struct formatter<aby::UUID> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        FmtContext::iterator format(const aby::UUID& uuid, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}", uuid.operator unsigned long long());
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

}


