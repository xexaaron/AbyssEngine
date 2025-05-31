#include "Platform/vk/VkInstance.h"
#include "Platform/vk/VkAllocator.h"
#include <stdlib.h>
#include "Core/Log.h"

namespace aby::vk {

    Instance::Instance() : 
        m_Inst(VK_NULL_HANDLE)
    {
    
    }

    Instance::Instance(const AppInfo& info, const std::vector<const char*>& extensions, const std::vector<const char*>& layers) :
        m_Inst(VK_NULL_HANDLE)
    {
        create(info, extensions, layers);
    }

    Instance::~Instance() {}

    void Instance::create(const AppInfo& info, const std::vector<const char*>& extensions, const std::vector<const char*>& layers) {
        auto missing_extensions = helper::are_ext_avail(extensions);
        if (!missing_extensions.empty()) {
            ABY_ERR("Missing {} required extension(s)", missing_extensions.size());
            for (const char* ext : missing_extensions) {
                ABY_ERR(" -- {}", ext);
            }
            auto avail_extensions = helper::get_extensions();
            ABY_LOG("Available Layer(s): {}", avail_extensions.size());
            for (auto ext : avail_extensions) {
                ABY_LOG(" -- {}", ext.extensionName);
            }

            ABY_ASSERT(false, "");
        }
        auto missing_layers = helper::are_layers_avail(layers); 
        if (!missing_layers.empty()) {
            ABY_ERR("Missing {} required layers(s)", missing_layers.size());
            for (const char* layer : missing_layers) {
                ABY_ERR(" -- {}", layer);
            }
            auto avail_layers = helper::get_layers();
            ABY_LOG("Available Layer(s): {}", avail_layers.size());
            for (auto layer : avail_layers) {
                ABY_LOG(" -- {} ({})", layer.layerName, layer.description);
            }
            ABY_ASSERT(false, "");
        }

        auto ver = VK_MAKE_VERSION(info.version.major, info.version.minor, info.version.patch);

        VkApplicationInfo ai = {};
        ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        ai.pApplicationName = info.name.c_str();
        ai.applicationVersion = ver;
        ai.pEngineName = info.name.c_str();
        ai.engineVersion = ver;
        ai.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        if (std::find(extensions.begin(), extensions.end(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != extensions.end()) {
            ci.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
        else {
            ci.flags = 0;
        }

        ci.pApplicationInfo = &ai;
        ci.enabledExtensionCount = static_cast<u32>(extensions.size());
        ci.ppEnabledExtensionNames = extensions.data();
        ci.enabledLayerCount = static_cast<u32>(layers.size());
        ci.ppEnabledLayerNames = layers.data();
        VK_CHECK(vkCreateInstance(&ci, IAllocator::get(), &m_Inst));
        ABY_DBG("vk::Instance::create");
        ABY_DBG("  App Version      {}.{}.{}", info.version.major, info.version.minor, info.version.patch);
        ABY_DBG("  Vulkan Version   1.3.0");
        ABY_DBG("  Extension(s)     {}", extensions.size());
        ABY_DBG("  Enabled Extensions: {}", extensions.size());
        for (std::size_t i = 0; i < extensions.size(); i++) {
            ABY_DBG("  ({}) -- {}", i + 1, extensions[i]);
        }
        ABY_DBG("  Enabled Layers(s)        {}", layers.size());
        for (std::size_t i = 0; i < layers.size(); i++) {
            ABY_DBG("  ({}) -- {}", i + 1, layers[i]);
        }

        pfn::load_functions(m_Inst);
    }

    void Instance::destroy() {
        vkDestroyInstance(m_Inst, IAllocator::get());
    }

    VkInstance Instance::inst() {
        return m_Inst;
    }

    Instance::operator VkInstance() {
        return m_Inst;
    }

}