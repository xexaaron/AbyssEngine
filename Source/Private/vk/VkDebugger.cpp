#include "vk/VkDebugger.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"

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
        dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        dbg.pfnUserCallback = msg_callback;
        dbg.pUserData = nullptr;
        VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &dbg, IAllocator::get(), &m_Debugger));
	}

    void Debugger::destroy() {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_Debugger, IAllocator::get());
    }

    Debugger::operator VkDebugUtilsMessengerEXT() {
        return m_Debugger;
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
                ABY_ERR("{}", callback_data->pMessage);
                break;
            }
            default:
                return VK_ERROR_UNKNOWN;
        }
        return VK_SUCCESS;
    }
}