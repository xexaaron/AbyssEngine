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

namespace fs = std::filesystem;

namespace aby {

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T>
    using Unique = std::unique_ptr<T>;

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

    struct AppVersion {
        std::uint32_t Major = 0;
        std::uint32_t Minor = 0;
        std::uint32_t Patch = 0;
    };

    enum class EBackend {
        VULKAN,
        DEFAULT = VULKAN,
    };

    struct AppInfo {
        // App name.
        std::string Name     = "App";
        // App version.
        AppVersion  Version  = { 0, 0, 0 };
        // Does the window inherit the name?
        bool        bInherit = true; 
        // Rendering backend.
        EBackend    Backend = EBackend::DEFAULT; 
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

    struct ConstexprPath {
    public:
        template <std::size_t N>
        constexpr ConstexprPath(const char(&data)[N]) : data(data), size(N - 1) {}

        constexpr bool operator==(const ConstexprPath& other) const {
            return other.data == data;
        }
        constexpr bool operator!=(const ConstexprPath& other) const {
            return !this->operator==(other);
        }

        // Compare if the path is a subpath of another (relative comparison)
        constexpr bool is_relative_to(const ConstexprPath& base) const {
            if (base.size == 0 || base.size > size) {
                return false;
            }
            for (std::size_t i = 0; i < base.size; ++i) {
                if (data[i] != base.data[i]) {
                    return false;
                }
            }
            return data[base.size] == '/';
        }

        constexpr bool file_name_eq(const ConstexprPath& path) const {
            std::size_t last_slash_pos = size;
            while (last_slash_pos > 0 && data[last_slash_pos - 1] != '/') {
                --last_slash_pos;

            }

            std::size_t file_name_size = size - last_slash_pos;

            // Compare each character of the file name
            for (std::size_t i = 0; i < file_name_size; ++i) {
                if (data[last_slash_pos + i] != path.data[i]) {
                    return false;  // Found a mismatch
                }
            }

            return true;  // All characters matched
        }

        template <std::size_t N = sizeof(__FILE__)>
        constexpr static inline bool source_filename_is(const ConstexprPath& name, const char(&file)[N] = __FILE__) {
            ConstexprPath path(file);
            return path.file_name_eq(name);
        }

        const char* data;
        std::size_t size;
    };

    template <typename T, T Min = std::numeric_limits<T>::min(), T Max = std::numeric_limits<T>::max()> requires std::is_arithmetic_v<T>
    class Random {
    public:
        static T Gen() noexcept {
            static std::random_device rand_device;
            static std::mt19937_64 generator(rand_device());

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
            return Random<T, Min, Max>::Gen();
        }
    };

    class UUID {
    public:
        UUID() : m_Value(Random<uint64_t>::Gen()) {}
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

    struct ETime {
        enum Enum {
            SECONDS,
            MILLISECONDS,
            MICROSECONDS,
            NANOSECONDS,
        };
        static std::string suffix_of(Enum value) {
            switch (value) {
                case SECONDS:		return "s";
                case MILLISECONDS:	return "ms";
                #pragma warning(push, 0)
                case MICROSECONDS:	return "μs";
                #pragma warning(pop)
                case NANOSECONDS:	return "ns";
                default: return "Unknown";
            }
            return "";

        }
        static std::string to_string(Enum value) {
            switch (value) {
                case SECONDS:		return "seconds";
                case MILLISECONDS:	return "milliseconds";
                case MICROSECONDS:	return "microseconds";
                case NANOSECONDS:	return "nanoseconds";
                default: return "Unknown";
            }
            return "";
        }
    };

    /* Base Unit : Seconds */
    struct Time {
    public:
        Time(float time = 0.f) : m_Time(time) {}
        Time(const Time& other) : m_Time(other.m_Time) {}
        Time(Time&& other) noexcept : m_Time(other.m_Time) {}

        operator float() const { return m_Time; }
        Time& operator=(float other) {
            m_Time = other;
            return *this;
        }

        float sec()	  const { return m_Time; }
        float milli() const { return m_Time * 1000.f; }
        float micro() const { return m_Time * 100000000.f; }
        float nano()  const { return m_Time * 1000000000.f; }

    private:
        float m_Time;
    };


}