#include "Core/Resource.h"
#include "Rendering/Context.h"

#ifdef _WIN32
#include <Windows.h>
#include <processthreadsapi.h>
#endif


namespace aby {

    Thread::~Thread() {
        if (m_Thread.joinable()) {
            m_Thread.join();
        }
    }

    void Thread::set_name(const wchar_t* name) {
        SetThreadDescription(reinterpret_cast<HANDLE>(m_Thread.native_handle()), name);
    }

    void Thread::join() {
        m_Thread.join();
    }

    void Thread::detach() {
        m_Thread.detach();
    }

    LoadThread::LoadThread(QueryResourceNextHandle query_next_handle) : 
        Thread([this]() { load(); }, L"Load Thread"),
        m_QueryNextHandle(std::move(query_next_handle)),
        m_FinishState(EFinishState::CONTINUE) 
    {
    }

    LoadThread::~LoadThread() {
        m_FinishState.store(EFinishState::LOAD_THEN_FINISH);
    }

    Resource LoadThread::add_task(EResource type, Task&& task) {
        std::lock_guard  lock_guard(m_Mutex);
        Resource::Handle next_handle   = m_QueryNextHandle(type);
        std::size_t      next_position = m_Tasks.count(type); 
        Resource::Handle handle        = static_cast<Resource::Handle>(next_position + next_handle);
        m_Tasks.emplace(type, std::move(task));
        return Resource{ type, handle };
    }
        
    std::size_t LoadThread::tasks() {
        return m_Tasks.size();
    }

    void LoadThread::sync() {
        // Finish loading resources.
        m_FinishState.store(EFinishState::LOAD_THEN_FINISH);
        join();
        // Restart the thread.
        if (!m_Thread.joinable()) {
            m_FinishState.store(EFinishState::CONTINUE);
            m_Thread = std::thread([this]() { load(); });
            set_name(L"Load Thread");
        }
    }

    void LoadThread::load() {
        EFinishState state = state = m_FinishState.load();
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
                    m_FinishState.store(EFinishState::FINISH);
                    break;
                case FINISH:
                    if (!m_Tasks.empty()) {
                        m_FinishState.store(EFinishState::LOAD_THEN_FINISH);
                    }
                    break;
                case CONTINUE:
                    break;
                default:
                    throw std::out_of_range("EFinishState");
                    break;
            }
        }

        std::cout << "Load Complete" << std::endl;
    }
}