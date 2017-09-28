#ifndef __UNITYGAME_SEMAPHORE_H_
#define __UNITYGAME_SEMAPHORE_H_
#include <mutex>
#include <atomic>

namespace UnityGame {
    class Semaphore {
    private:
        std::atomic<bool>         m_Available;
        long                      m_AvailableResource;
        std::mutex                m_MutexLock;
        std::condition_variable   m_ConditionLock;

        friend bool CheckCondition(Semaphore* sem);
        void WaitResource();
    public:
        Semaphore();
        Semaphore(long ResourceNumber);
        ~Semaphore();
        void DisableAndAwakeAll();
        bool Wait();
        bool Post();
    };
}
#endif //__UNITYGAME_SEMAPHORE_H_