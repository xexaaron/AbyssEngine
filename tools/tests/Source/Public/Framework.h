#pragma once
#include <vector>
#include <string_view>
#include <iostream>
#include <format>

#define TEST(test_name)                                                          \
namespace test_name {                                                            \
    static auto eval() -> bool;                                                  \
    class test_name;                                                             \
    class test_name : public aby::Test {                                         \
    public:                                                                      \
        test_name() {                                                            \
            aby::TestFramework::get().add(this);                                 \
        }                                                                        \
        auto operator()() -> bool override {                                     \
            return eval();                                                       \
        }                                                                        \
        auto name() -> std::string_view override {                               \
            return #test_name;                                                   \
        }                                                                        \
        template <typename... Args>                                              \
        static void err(std::format_string<Args...> fmt, Args&&... args) {       \
            std::cerr << std::format("[Test:{}] [Error] {}", #test_name, std::format(fmt, args...)) << '\n'; \
        }                                                                        \
        static auto err(const std::string& str) -> void {                        \
            std::cerr << std::format("[Test:{}] [Error] {}", #test_name, str) << '\n'; \
        }                                                                        \
    };                                                                           \
    static test_name Instance;                                                   \
}                                                                                \
auto test_name::eval() -> bool        

namespace aby {

    class Test {
    public:
        Test() = default;
        Test(Test&) = delete;
        Test(Test&&) noexcept = delete;
        virtual ~Test() = default;

        virtual auto name() -> std::string_view = 0;

        virtual auto operator()() -> bool = 0;
    private:
    };

    class TestFramework {
    public:
        static TestFramework& get();

        void add(Test* test);

        bool run();
    private:
        std::vector<Test*> m_Tests;
    };

}


