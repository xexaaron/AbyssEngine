#include "Framework.h"
#include <chrono>

namespace aby {

    TestFramework& aby::TestFramework::get() {
        static TestFramework fw;
        return fw;
    }

    void aby::TestFramework::add(Test* test) {
        m_Tests.push_back(std::move(test));
    }

    bool aby::TestFramework::run() {
        const int   result_width     = 8;
        const int   time_width       = 11;
        const auto  underline_on     = "\033[4m";
        const auto  underline_off    = "\033[24m";
        const auto  bold_on          = "\033[1m";
        const auto  bold_off         = "\033[22m";
        const auto  color_cyan       = "\033[36m";
        const auto  color_reset      = "\033[0m";
        const auto  color_number     = "\033[38;2;164;206;168m";
        const auto  color_string     = "\033[38;2;214;157;133m";
        bool        success          = true;
        std::size_t passed           = 0;
        std::size_t max_label_len    = 0;

        for (const auto& test : m_Tests)
            max_label_len = std::max(max_label_len, test->name().size());

        auto repeat = [](auto ch, std::size_t count) -> std::string {
            std::string result;
            for (std::size_t i = 0; i < count; ++i)
                result.append(ch);
            return result;
            };
        auto print_top = [&]() {
            std::cout << "┌" << repeat("─", max_label_len + 2)
                << "┬" << repeat("─", result_width + 2)
                << "┬" << repeat("─", time_width + 2)
                << "┐\n";
            };
        auto print_separator = [&]() {
            std::cout << "├" << repeat("─", max_label_len + 2)
                << "┼" << repeat("─", result_width + 2)
                << "┼" << repeat("─", time_width + 2)
                << "┤\n";
            };
        auto print_bottom = [&]() {
            std::cout << "└" << repeat("─", max_label_len + 2)
                << "┴" << repeat("─", result_width + 2)
                << "┴" << repeat("─", time_width + 2)
                << "┘\n";
            };
        auto print_header_col = [&](const std::string& text, int width, const char* color_code) {
            int visible_len = static_cast<int>(text.size());
            int total_padding = std::max(0, width - visible_len);
            int left_padding = total_padding / 2;
            int right_padding = total_padding - left_padding;

            return std::string(color_code) + std::string(left_padding, ' ') +
                underline_on + bold_on + text + + bold_off + underline_off +
                std::string(right_padding, ' ') + color_reset;
        };
        auto print_row = [&](auto name, auto result_str, auto time_str) {
            std::cout << std::format("│ {}{:<{}}{} │ {} │ {} │\n", color_string, name, max_label_len, color_reset, result_str, time_str);
        };

        print_top();
        std::cout << std::format("│ {} │ {} │ {} │\n",
            print_header_col("Test", max_label_len, color_string),
            print_header_col("Result", result_width, color_cyan),
            print_header_col("Time (ns)", time_width, color_number)
        );
        print_separator();

        auto total_time_start = std::chrono::steady_clock::now();
        auto total_time_end   = total_time_start;

        for (auto& test : m_Tests) {
            auto start = std::chrono::steady_clock::now();
            bool result = (*test)();
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            total_time_end += duration;
            success &= result;
            passed += result;

            std::string result_label = result ? "Success" : "Failure";
            std::string padded_result_label = std::format("{:<{}}", result_label, result_width);
            std::string result_str = result
                ? ("\033[32m" + padded_result_label + "\033[0m")
                : ("\033[31m" + padded_result_label + "\033[0m");
            std::string time_str = std::format("{}{}ns{}", color_number,
                std::format("{:>{}}", duration.count(), time_width - 2), color_reset);

            print_row(test->name(), result_str, time_str);

            if (test != m_Tests.back())
                print_separator();
        }

        auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(total_time_end - total_time_start);

        print_separator();

        std::string passed_text = std::format("{}/{}", passed, m_Tests.size());
        std::string padded_passed = std::format("{:<{}}", passed_text, result_width);
        std::string colored_passed = (passed == m_Tests.size())
            ? "\033[32m" + padded_passed + "\033[0m"
            : "\033[31m" + padded_passed + "\033[0m";

        std::string time_str = std::format("{}{}ns{}", color_number,
            std::format("{:>{}}", total_duration.count(), time_width - 2), color_reset);

        print_row("Total", colored_passed, time_str);

        print_bottom();

        return success;
    }

}