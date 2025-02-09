#include "Rendering/Renderer.h"
#include "vk/VkRenderer.h"

namespace aby {
    
    Ref<Renderer> Renderer::create(Ref<Context> ctx) {
        switch (ctx->backend()) {
            case EBackend::VULKAN: {
                Ref<vk::Context> vk_ctx = std::static_pointer_cast<vk::Context>(ctx);
                return create_ref<vk::Renderer>(vk_ctx);
            }
            default:
                ABY_ASSERT(false, "Context::backend() out of range");
        }
        return nullptr;
    }

}
