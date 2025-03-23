#pragma once

#include "Core/Resource.h"
#include "Core/Log.h"
#include <mutex>
#include <thread>
#include <condition_variable>

namespace aby {

    class Thread {
    public:
        template <typename Fn> requires (!std::is_same_v<std::decay_t<Fn>, Thread>)
        explicit Thread(Fn&& fn, const std::string& thread_name = "") : m_Thread(std::forward<Fn>(fn)) {
            set_name(thread_name);
            ABY_LOG("Thread::Thread: {}", thread_name);
        }

        virtual ~Thread();

        void set_name(const std::string& name);

        void join();
        void detach();
    protected:
        std::thread m_Thread;
    };

    class LoadThread final : public Thread {
    private:
        enum class EFinishState {
            FINISH           = 0,
            LOAD_THEN_FINISH = 1,
            CONTINUE         = 2,
        };
    public:
        using QueryResourceNextHandle = std::function<Resource::Handle(EResource)>;
        using Task = std::function<void()>;

        explicit LoadThread(QueryResourceNextHandle query_next_handle);
        ~LoadThread();

        Resource add_task(EResource type, Task&& task);
        std::size_t tasks() const;
        void sync();
    private:
        void load();
    public:
        QueryResourceNextHandle        m_QueryNextHandle;
        std::multimap<EResource, Task> m_Tasks;
        // Recursive mutex is necessary due
        // to fonts and how they are loaded.
        // The font needs to be loaded, then 
        // a texture associated with a font needs
        // to be loaded within the same scope as the font
        // creation.
        std::recursive_mutex           m_Mutex;
        std::atomic<EFinishState>      m_FinishState;
        std::condition_variable m_CondVar;
        std::mutex m_CondMutex;
    };
}
