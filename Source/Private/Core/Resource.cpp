#include "Core/Resource.h"
#include "Rendering/Context.h"

#ifdef _WIN32
#include <Windows.h>
#include <processthreadsapi.h>
#endif


namespace aby {

    ResourceThread::ResourceThread(Context* ctx) :
        bStopLoading(false),
        bFinishUp(false),
        m_Ctx(ctx),
        m_LoadThread([this](){ this->ResourceThread::executor(); }),
        m_CV(),
        m_Mutex(),
        m_ResourceData(static_cast<std::size_t>(EResource::MAX_ENUM)) // Reserve max_enum entries.
    {
    #ifdef _WIN32
        WIN32_CHECK(SetThreadDescription(m_LoadThread.native_handle(), L"Resource Thread"));
    #endif
    }

    Resource ResourceThread::add_task(EResource type, const Task& task) {
        // ------------------------------------------------------------------
        // Say we have 5 or 0...4 in queue, so the next position is 5;
        // The context has 3 or 0...2 resources of this type already so the next handle is 3. 
        // So the next expected resource handle for this resource
        // should be position + next_handle
        // This way we can return a valid object 
        // before its reference data has been loaded.
        // 
        // Resources are always created using T::create(...),
        // sending a task to the resource manager if asynchronously loaded
        // else loading the resource then adding it to the context
        // immediately
        // -------------------------------------------------------------------
        std::lock_guard  lock_guard(m_Mutex);
        Resource::Handle next_handle   = 0; 
        std::size_t      next_position = m_ResourceData[type].LoadQueue.size();
        switch (type) {
            case EResource::SHADER:
                next_handle = m_Ctx->shaders().size();
                break;
            case EResource::TEXTURE:
                next_handle = m_Ctx->textures().size();
                break;
            case EResource::FONT:
                next_handle = m_Ctx->fonts().size();
                break;
            default:
                throw std::runtime_error("Invalid EResource type");
        }
        Resource::Handle handle = next_position + next_handle;
        m_ResourceData[type].LoadQueue.push(task);
        return Resource(type, handle);
    }

    void ResourceThread::sync() {
        if (m_LoadThread.joinable()) {
            bFinishUp = true;
            m_LoadThread.join();
        }
    }

    void ResourceThread::executor() {
        while (!bStopLoading) {  // Exit if either stop or finish up is true
            std::unique_lock<std::mutex> lock(m_Mutex);

            bool all_empty = true;
            for (auto& [resource_type, resource_data] : m_ResourceData) {
                if (!resource_data.LoadQueue.empty()) {
                    Task task = std::move(resource_data.LoadQueue.front());
                    resource_data.LoadQueue.pop();
                    task();
                    all_empty = false;
                }
            }
            if (bFinishUp) {
                bStopLoading = true;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep if necessary
        }
    }

}