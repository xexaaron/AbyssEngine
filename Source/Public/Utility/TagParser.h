#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <cstring>
#include <algorithm>

namespace aby::util {

    enum class ETextDecor {
        NONE = 0,
        UNDERLINE,
        HIGHLIGHT,
    };

    enum class ETagComparison {
        NONE,
        OPEN,
        CLOSE
    };

    struct Range {
        Range(std::size_t start = std::string::npos, std::size_t end = std::string::npos);
        explicit operator bool() const;

        std::size_t start;
        std::size_t end;
    };

    struct TextDecor {
        TextDecor(ETextDecor type = ETextDecor::NONE, Range range = {});

        ETextDecor type;
        Range      range;
    };

    struct Tag {
        Tag(ETextDecor type, const char(&id)[3]);

        std::size_t len(ETagComparison comparison);
        static std::vector<Tag> tags();

        ETextDecor type;
        std::string open;
        std::string close;
    };

    struct DecorStackEntry {
        DecorStackEntry(ETextDecor decor, ETagComparison comparison, std::size_t pos);

        ETextDecor     decor_type;
        ETagComparison tag_type;
        std::size_t    position;
    };

    std::vector<TextDecor> parse_and_strip_tags(std::string& text);

}

namespace std {
    string to_string(aby::util::ETagComparison compare);
    string to_string(aby::util::ETextDecor type);
}