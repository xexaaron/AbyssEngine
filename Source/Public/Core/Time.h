#pragma once
#include <chrono>
#include <string>

namespace aby {
    
    enum class ETime {
        SECONDS,
        MILLISECONDS,
        MICROSECONDS,
        NANOSECONDS,
    };

    /* Base Unit : Seconds */
    class Time {
    public:
        Time(float time = 0.f);
        Time(const Time& other);
        Time(Time&& other) noexcept;

        float sec()	  const;
        float milli() const;
        float micro() const;
        float nano()  const;

        operator float() const;
        Time& operator=(float other);
    private:
        float m_Time;
    };

    class Timer {
    public:
        Timer();
        Timer(const Timer&) = delete;
        Timer(Timer&&) noexcept = delete;
        ~Timer() = default;

        Timer operator=(const Timer&) = delete;
        Timer operator=(Timer&&) = delete;

        void reset();
        Time elapsed();
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };

}

namespace std {
    string to_string(aby::ETime time);
}