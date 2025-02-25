#pragma once
#include <vector>
#include <memory>
#include <string_view>
#include <iostream>

#define TEST(test_name)                                                          \
namespace test_name {                                                            \
    static auto eval() -> bool;                                                  \
    class test_name;                                                             \
    using T = test_name;                                                         \
    class test_name : public aby::Test {                                         \
    public:                                                                      \
        test_name() {                                                            \
            aby::TestFramework::get().add(std::make_unique<T>());                \
        }                                                                        \
        auto operator()() -> bool override {                                     \
            return eval();                                                       \
        }                                                                        \
        auto name() -> std::string_view override {                               \
            return #test_name;                                                   \
        }                                                                        \
    };                                                                           \
    static T Instance;                                                           \
}                                                                                \
auto test_name::eval() -> bool        


namespace aby {

    class Test {
    public:
        virtual auto operator()() -> bool = 0;
        virtual auto name() -> std::string_view = 0;
    private:
    };

    class TestFramework {
    public:
        static TestFramework& get() {
            static TestFramework fw;
            return fw;
        }

        void add(std::unique_ptr<Test> test) {
            m_Tests.push_back(std::move(test));
        }

        bool run() {
            bool success = true;
            for (auto& test : m_Tests) {
                bool result = (*test)();
                success &= result;
                std::string result_str = result ? "Success" : "Failure";
                std::cout << "[test] [" << test->name() << "] " << result << '\n';
            }
            return success;
        }
    private:
        std::vector<std::unique_ptr<Test>> m_Tests;
    };

}

TEST(foo) {
    return false;
}

