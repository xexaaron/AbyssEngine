#include "Core/Thread.h"

#include "Core/App.h"
#include "Platform/Platform.h"




namespace aby {

    Thread::~Thread() {
        if (m_Thread.joinable()) {
            m_Thread.join();
        }
    }

    void Thread::set_name(const std::string& name) {
        if (!sys::set_thread_name(m_Thread, name)) {
            ABY_ERR("Failed to set thread name");
        }
    }

    void Thread::join() {
        m_Thread.join();
    }

    void Thread::detach() {
        m_Thread.detach();
    }

    LoadThread::LoadThread(QueryResourceNextHandle query_next_handle) :
        Thread([this]() { load(); }, "Load Thread"),
        m_QueryNextHandle(std::move(query_next_handle)),
        m_FinishState(EFinishState::CONTINUE) {
    }

    LoadThread::~LoadThread() {
        m_FinishState.store(EFinishState::LOAD_THEN_FINISH, std::memory_order_release);
        // ~Thread will join it.
    }

    Resource LoadThread::add_task(EResource type, Task&& task) {
        std::lock_guard  lock_guard(m_Mutex);
        Resource::Handle next_handle = m_QueryNextHandle(type);
        std::size_t      next_position = m_Tasks.count(type);
        Resource::Handle handle = static_cast<Resource::Handle>(next_position + next_handle);
        m_Tasks.emplace(type, std::move(task));
        ABY_DBG("LoadThread::add_task(...) Resource[ type: {}, handle: {} ]", static_cast<int>(type), handle);
        return Resource{ type, handle };
    }

    std::size_t LoadThread::tasks() {
        std::lock_guard lock(m_Mutex);
        return m_Tasks.size();
    }

    void LoadThread::sync() {
        // Finish loading resources.
        m_FinishState.store(EFinishState::LOAD_THEN_FINISH, std::memory_order_release);
        join();
        // Restart the thread.
        if (!m_Thread.joinable()) {
            m_FinishState.store(EFinishState::CONTINUE, std::memory_order_release);
            m_Thread = std::thread([this]() { load(); });
            set_name("Load Thread");
        }
    }

    void LoadThread::load() {
        EFinishState state = state = m_FinishState.load(std::memory_order_acquire);
        while (state != EFinishState::FINISH) {
            state = m_FinishState.load();
            {
                std::lock_guard lock(m_Mutex);
                if (!m_Tasks.empty()) {
                    auto it = m_Tasks.begin();
                    Task task = std::move(it->second);
                    m_Tasks.erase(it);
                    task();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Prevent busy-wait

            switch (state) {
                using enum EFinishState;
                case LOAD_THEN_FINISH:
                    m_FinishState.store(EFinishState::FINISH, std::memory_order_release);
                    break;
                case FINISH:
                    return;
                case CONTINUE:
                    break;
                default:
                    ABY_WARN("Load Thread was left an invalid state.");
                    m_FinishState.store(EFinishState::CONTINUE, std::memory_order_release);
                    break;
            }
        }
    }

}
