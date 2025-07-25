#pragma once

#include <format>
#include <iostream>
#include <mutex>
#include <source_location>
#include <filesystem>
#include <functional>
#include <vector>
#include <chrono>
#include <queue>
#include <glm/glm.hpp>
#include "Core/Common.h"
#include "Utility/Random.h"

namespace aby {

    enum class ELogColor {
        Grey      = 0,
        Cyan      = 36,
        Yellow    = 33,
        Red       = 91,
    };

    enum class ELogLevel {
        LOG    = 0,
        DEBUG  = 36,
        WARN   = 33,
        ERR    = 91,
        ASSERT = ERR,
    };

    struct LogMsg {
        ELogLevel   level;
        std::string text;
        ELogColor   color() const;
    };

    struct LogCfg {
        bool          only_do_cb = DEBUG_FALSE; // Disable logging to stdout, but process callbacks.
        bool          buffered   = DEBUG_FALSE; // Disable logging immediately, only log after frame end.
        std::ostream* cout       = &std::cout;  // Log output stream
        std::ostream* cerr       = &std::cerr;  // Error output stream
    };

    class Logger {
    private:
        template <typename... Args>
        inline static void print(const std::string& context, ELogColor  color, std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard lock(m_Mutex);
            std::string prefix = std::format("{}[{}]", time_date_now_header(), context);
            std::string msg    = std::format(fmt, std::forward<Args>(args)...);
            std::string out    = prefix + "   " + msg;
            if (m_Cfg.buffered) {
                m_MsgBuff.emplace(static_cast<ELogLevel>(color), out);
            }
            else {
                if (!m_Cfg.only_do_cb) {
                    switch (static_cast<ELogLevel>(color)) {
                    case ELogLevel::LOG:
                    case ELogLevel::DEBUG:
                        *m_Cfg.cout << "\033[" << static_cast<int>(color) << "m" << out << "\033[0m" << '\n';
                        break;
                    case ELogLevel::WARN:
                    case ELogLevel::ERR:
                        *m_Cfg.cerr << "\033[" << static_cast<int>(color) << "m" << out << "\033[0m" << '\n';
                        break;
                    }
                }
                for (auto& cb : m_Callbacks) {
                    cb(LogMsg{ static_cast<ELogLevel>(color), out });
                }
            }
        }
    public:
        using Callback = std::function<void(const LogMsg&)>;

        static void        set_cfg(const LogCfg& cfg);
        static void        flush();
        static std::size_t add_callback(Callback&& callback);
        static void        remove_callback(std::size_t idx);
        static std::string time_date_now_header();
        static std::string time_date_now();
        static glm::vec4   log_color_to_vec4(ELogColor color);

        template <typename... Args>
        static void log(std::format_string<Args...> fmt, Args&&... args) {
            print("LOG", ELogColor::Grey, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void warn(std::format_string<Args...> fmt, Args&&... args) {
            print("WRN", ELogColor::Yellow, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void error(std::format_string<Args...> fmt, Args&&... args) {
            print("ERR", ELogColor::Red, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void Assert(std::format_string<Args...> fmt, Args&&... args) {
        #ifndef NDEBUG
            print("AST", ELogColor::Red, fmt, std::forward<Args>(args)...);
        #endif
        }
        template <typename... Args>
        static void debug(std::format_string<Args...> fmt, Args&&... args) {
        #ifndef NDEBUG
            print("DBG", ELogColor::Cyan, fmt, std::forward<Args>(args)...);
        #endif
        }
    private:
        static inline std::vector<Callback> m_Callbacks = {};
        static inline std::recursive_mutex  m_Mutex     = {};
        static inline std::queue<LogMsg>    m_MsgBuff   = {};
        static inline LogCfg                m_Cfg       = {};
    };

} 

namespace std {
    template <>
    struct formatter<source_location> {
        template<class ParseContext>
        constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }
        
        template <typename FmtContext>
        typename FmtContext::iterator format(const source_location& loc, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}:{} {}", loc.file_name(), loc.line(), loc.function_name());
        }
    };

    template <>
    struct formatter<filesystem::path> {
        template<class ParseContext>
        constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        typename FmtContext::iterator format(const filesystem::path& path, FmtContext& ctx) const {
            return format_to(ctx.out(), "<fp>\"{}\"</fp>", path.generic_string());
        }
    };

    template <>
    struct formatter<aby::util::UUID> {
        template<class ParseContext>
        constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        typename FmtContext::iterator format(const aby::util::UUID& uuid, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}", uuid.operator u64());
        }
    };

    template <glm::length_t L, typename T, glm::qualifier Q>
    struct formatter<glm::vec<L, T, Q>>  {
        template <typename ParseContext>
        constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        typename FmtContext::iterator format(const glm::vec<L, T, Q>& v, FmtContext& ctx) const {
            std::string out = "(";
            for (glm::length_t i = 0; i < L; ++i) {
                out += std::to_string(v[i]);
                if (i < L - 1) {
                    out += ", ";
                }
            }
            out += ")";
            return format_to(ctx.out(), "{}", out);
        }


    };

}

namespace aby {
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


