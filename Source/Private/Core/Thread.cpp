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
        if (m_Thread.joinable()) {
            m_Thread.join();
        }
    }

    void Thread::detach() {
        m_Thread.detach();
    }

    LoadThread::LoadThread(QueryResourceNextHandle query_next_handle) :
        Thread([this]() { load(); }, "Load Thread"),
        m_QueryNextHandle(std::move(query_next_handle)),
        m_FinishState(EFinishState::CONTINUE) 
    {

    }

    LoadThread::~LoadThread() {
        m_FinishState.store(EFinishState::FINISH, std::memory_order_release);
        m_Thread.detach();
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
        return m_Tasks.size();
    }

    void LoadThread::sync() {
        if (m_Tasks.empty() || m_FinishState.load(std::memory_order_acquire) == EFinishState::FINISH) return; 
        std::lock_guard lock(m_Mutex);
        while (!m_Tasks.empty()) {
            auto it = m_Tasks.begin();
            Task task = it->second;
            m_Tasks.erase(it);
            task();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m_FinishState.store(EFinishState::FINISH, std::memory_order_release);
        m_Tasks.clear();
    }

    void LoadThread::load() {
        while (m_FinishState.load(std::memory_order_acquire) != EFinishState::FINISH) {
            std::lock_guard lock(m_Mutex);
            if (!m_Tasks.empty()) {
                auto it = m_Tasks.begin();
                Task task = std::move(it->second);
                m_Tasks.erase(it);
                task();
            }
            switch (m_FinishState.load(std::memory_order_acquire)) {
                using enum EFinishState;
                case FINISH: 
                    return;
                case CONTINUE:
                    continue;
                default:
                    ABY_WARN("Load Thread was left an invalid state.");
                    m_FinishState.store(EFinishState::CONTINUE, std::memory_order_release);
                    break;
            }
        }
    }

}
