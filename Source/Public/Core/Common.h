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

#define EXPAND_VEC2(v) v.x, v.y
#define EXPAND_VEC3(v) v.x, v.y, v.z
#define EXPAND_VEC4(v) v.x, v.y, v.z, v.w
#define EXPAND_COLOR(c) c.r, c.g, c.b, c.a
#define WIN32_CHECK(x) do { \
        HRESULT r = (x); \
        if (FAILED(r)) { \
            ABY_ASSERT(false, "[win32] API call failed with HRESULT: 0x{:X}", r); \
        } \
    } while(0)


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

    class Timer {
    public:
        Timer() {
            reset();
        }

        ~Timer() {
           
        }

        void reset() {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        Time elapsed() {
            using clock = std::chrono::high_resolution_clock;
            using ns = std::chrono::nanoseconds;

            float elapsed_seconds =
                std::chrono::duration_cast<ns>(clock::now() - m_Start).count()
                * 0.001f * 0.001f * 0.001f;

            return Time(elapsed_seconds);
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };



}