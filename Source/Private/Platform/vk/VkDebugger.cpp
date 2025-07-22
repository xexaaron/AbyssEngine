#include "Platform/vk/VkDebugger.h"
#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"

#include <regex>

namespace aby::vk {
    Debugger::Debugger() : 
        m_Instance(VK_NULL_HANDLE),
        m_Debugger(VK_NULL_HANDLE)
    {

    }

    Debugger::Debugger(Instance& instance) : 
        m_Instance(VK_NULL_HANDLE), 
        m_Debugger(VK_NULL_HANDLE)
    {
        create(instance);
    }

    Debugger::~Debugger() {
    }

	void Debugger::create(Instance& instance) {
        m_Instance = instance;
        VkDebugUtilsMessengerCreateInfoEXT dbg = {};
        dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg.messageSeverity = 
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg.messageType = 
            // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        dbg.pfnUserCallback = msg_callback;
        dbg.pUserData = nullptr;
        VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &dbg, IAllocator::get(), &m_Debugger));
        ABY_DBG("vk::Debugger::create");
	}

    void Debugger::destroy() {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_Debugger, IAllocator::get());
        ABY_DBG("vk::Debugger::destroy");
    }

    Debugger::operator VkDebugUtilsMessengerEXT() {
        return m_Debugger;
    }
    
    struct ValidationError {
        std::string context;
        std::string object;
        std::string description;
        std::optional<std::string> reference;

        static std::string wrap_text(const std::string& text, std::size_t width = 60) {
            std::ostringstream wrapped;
            std::istringstream words(text);
            std::string word;
            std::string line;

            while (words >> word) {
                if (line.length() + word.length() + 1 > width) {
                    wrapped << line << "\n                "; // indented for readability
                    line = word;
                }
                else {
                    if (!line.empty()) line += " ";
                    line += word;
                }
            }

            if (!line.empty())
                wrapped << line;

            return wrapped.str();
        }

        std::string format() const {
            std::ostringstream oss;
            oss << "Validation Error: {\n"
                << "  Context:     [ " << context << " ],\n"
                << "  Object:      [ " << object << " ]\n"
                << "  Description: " << wrap_text(description) << "\n";
            if (reference.has_value())
                oss << "  References:  (" << reference.value() << ")\n";
            oss << "}";
            return oss.str();
        }
    };

    ValidationError parse_validation_error(const std::string& raw) {
        ValidationError err;

        std::regex context_re(R"(Validation Error: \[ ([^\]]+) \])");
        std::regex object_re(R"(Object \d+: handle = ([^,]+), type = ([^;]+);)");
        std::regex desc_re(R"(\| MessageID = .* \| (.+?)(?: The Vulkan spec states:|$))");
        std::regex ref_re(R"(https?://[^\s\)]+)");

        std::smatch match;

        if (std::regex_search(raw, match, context_re))
            err.context = match[1].str();

        if (std::regex_search(raw, match, object_re))
            err.object = "ID: 0, Handle: " + match[1].str() + ", Type: " + match[2].str();

        if (std::regex_search(raw, match, desc_re))
            err.description = match[1].str();

        if (std::regex_search(raw, match, ref_re))
            err.reference = match[0].str();

        return err;
    }

    VkBool32 Debugger::msg_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
        switch (type) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: {
                ABY_LOG("{}", callback_data->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
                ABY_WARN("{}", callback_data->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
                ValidationError err = parse_validation_error(callback_data->pMessage);
                ABY_ERR("{}", err.format());
                break;
            }
            default:
                return static_cast<VkBool32>(VK_ERROR_UNKNOWN);
        }
        return static_cast<VkBool32>(VK_SUCCESS);
    }
}