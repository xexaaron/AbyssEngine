#include "Utility/TagParser.h"

namespace aby::util {
	Range::Range(std::size_t start, std::size_t end) : start(start), end(end) {}
	
    Range::operator bool() const { return start != std::string::npos && end != std::string::npos; }

    TextDecor::TextDecor(ETextDecor type, Range range) : type(type), range(range) {}
    
    Tag::Tag(ETextDecor type, const char(&id)[3]) : type(type), open({ '<', id[0], id[1], '>' }), close{ '<', '/', id[0], id[1], '>' } {}
    
    DecorStackEntry::DecorStackEntry(ETextDecor decor, ETagComparison comparison, std::size_t pos) : decor_type(decor), tag_type(comparison), position(pos) {}

    std::size_t Tag::len(ETagComparison comparison) {
        switch (comparison) {
            case ETagComparison::NONE:  return 0;
            case ETagComparison::OPEN:  return open.size();
            case ETagComparison::CLOSE: return close.size();
            default:
                throw std::runtime_error("Unimplemented Case");
        }
    }

    std::vector<Tag> Tag::tags() {
        return {
            Tag(ETextDecor::UNDERLINE, "ul"),
            Tag(ETextDecor::HIGHLIGHT, "hl"),
        };
    }

    std::vector<TextDecor> parse_and_strip_tags(std::string& text) {
        std::vector<Tag>             tags = Tag::tags();
        std::vector<DecorStackEntry> stack;
        std::vector<TextDecor>       decors;
        std::vector<DecorStackEntry> filtered_stack;
        std::size_t                  cursor = 0;

        while (cursor < text.size()) {
            bool matched = false;
            for (auto& tag : tags) {
                for (ETagComparison comparison : {ETagComparison::OPEN, ETagComparison::CLOSE}) {
                    std::string& tag_str = (comparison == ETagComparison::OPEN) ? tag.open : tag.close;
                    if (text.compare(cursor, tag_str.size(), tag_str) == 0) {
                        stack.emplace_back(tag.type, comparison, cursor);
                        text.erase(cursor, tag_str.size());
                        matched = true;
                        break;
                    }
                }
                if (matched) break;
            }
            if (!matched) {
                ++cursor;
            }
        }

        for (auto& entry : stack) {
            if (entry.tag_type == ETagComparison::OPEN) {
                filtered_stack.push_back(entry);
            }
            else if (entry.tag_type == ETagComparison::CLOSE) {
                auto it = std::find_if(filtered_stack.rbegin(), filtered_stack.rend(),
                    [&](const DecorStackEntry& e) {
                        return e.decor_type == entry.decor_type && e.tag_type == ETagComparison::OPEN;
                    });
                if (it != filtered_stack.rend()) {
                    decors.emplace_back(entry.decor_type, Range(it->position, entry.position - 1));
                    filtered_stack.erase(std::next(it).base());
                }
            }
        }
        return decors;
    }
}

std::string std::to_string(aby::util::ETagComparison compare) {
    switch (compare) {
        case aby::util::ETagComparison::OPEN:  return "OPEN";
        case aby::util::ETagComparison::CLOSE: return "CLOSE";
        default: return "NONE";
    }
}

std::string std::to_string(aby::util::ETextDecor type) {
    switch (type) {
        case aby::util::ETextDecor::UNDERLINE: return "UNDERLINE";
        case aby::util::ETextDecor::HIGHLIGHT: return "HIGHLIGHT";
        default: return "NONE";
    }
}
