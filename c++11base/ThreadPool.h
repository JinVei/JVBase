#ifndef __JVBASE_THREADPOOL_H_
#define __JVBASE_THREADPOOL_H_

#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include <list>

#include "Semaphore.h"

namespace JVBase {

    class ThreadPool {
        typedef std::shared_ptr<std::thread> thread_ptr;
        typedef std::function<void()> callback_ptr;
    private :
        std::atomic<bool>        m_bRun;
        unsigned int             m_nThreadsNum;
        Semaphore                m_TaskSemaphore;
        std::atomic_flag         m_TaskQueuesSpinlock;
        std::list<thread_ptr>  m_vptrThreads;
        std::queue<callback_ptr> m_vTaskQueue;

        friend int WorkThreadFuction(ThreadPool* threadPool);
        int InitWorkThreads();
    public:
        ThreadPool();
        ThreadPool(unsigned int nThreadNum);
        ~ThreadPool();
        void PushTask(callback_ptr fn);
        void DestroyThreadPool();
        void CreateThreadAndPushTask(callback_ptr fn);
    };
}

#endif // __JVBASE_THREADPOOL_H_