#pragma once
#include <chrono>
#include <source_location>
#include "Core/Time.h"
#include "Core/Common.h"

namespace aby {
    class App;
}

#define PROFILE_SCOPE(label) ::aby::util::ProfileResult CONCAT(__profile_result_, __LINE__)(label, std::source_location::current())

namespace aby::util {

    class ProfileResult {
    public:
        ProfileResult(const std::string& label, const std::source_location& source);
        ~ProfileResult(); 

        std::string          label;
        std::source_location source;
        Time                 duration;
    private:
        std::chrono::high_resolution_clock::time_point m_Start;
    };

    class Profiler {
    public:
        static Profiler& get();
        void set_app(App* app);
        void profile(const ProfileResult& result);
    private:
        App* m_App;
    };

}
