#include "Core/Time.h"

namespace std {
    string to_string(aby::ETime value) {
        switch (value) {
            using enum aby::ETime;
            case SECONDS:		return "seconds";
            case MILLISECONDS:	return "milliseconds";
            case MICROSECONDS:	return "microseconds";
            case NANOSECONDS:	return "nanoseconds";
            default: 
                return "Unknown";
        }
        return "UNREACHABLE: ETime";
    }
}

namespace aby {


    Time::Time(float time) : m_Time(time) {}
    Time::Time(const Time& other) : m_Time(other.m_Time) {}
    Time::Time(Time&& other) noexcept : m_Time(other.m_Time) {}

    Time::operator float() const { return m_Time; }
    Time& Time::operator=(float other) {
        m_Time = other;
        return *this;
    }

    float Time::sec()	const { return m_Time; }
    float Time::milli() const { return m_Time * 1000.f; }
    float Time::micro() const { return m_Time * 100000000.f; }
    float Time::nano()  const { return m_Time * 1000000000.f; }


    Timer::Timer() {
        reset();
    }


    void Timer::reset() {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    Time Timer::elapsed() {
        using clock = std::chrono::high_resolution_clock;
        using ns = std::chrono::nanoseconds;

        float elapsed_seconds =
            std::chrono::duration_cast<ns>(clock::now() - m_Start).count()
            * 0.001f * 0.001f * 0.001f;

        return Time(elapsed_seconds);
    }

}