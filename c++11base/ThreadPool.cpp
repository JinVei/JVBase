
#include "ThreadPool.h"

using namespace std;
JVBase::ThreadPool::ThreadPool()
{
    m_bRun = true;
    m_nThreadsNum = 1;
    this->InitWorkThreads();
}

JVBase::ThreadPool::ThreadPool(unsigned int nThreadNum)
{
    m_bRun = true;
    m_nThreadsNum = nThreadNum;
    this->InitWorkThreads();
}

JVBase::ThreadPool::~ThreadPool()
{
    if (m_bRun)
    {
        this->DestroyThreadPool();
    }
}
void JVBase::ThreadPool::DestroyThreadPool()
{
    m_bRun = false;
    m_TaskSemaphore.DisableAndAwakeAll();
    for (auto& thread : m_vptrThreads)
    {
        thread->join();
    }
    m_vptrThreads.clear();
}
static int JVBase::WorkThreadFuction(JVBase::ThreadPool* threadPool)
{
    int nRetCode;
    while (threadPool->m_bRun)
    {
        nRetCode = threadPool->m_TaskSemaphore.Wait();
        
        if (!threadPool->m_bRun || !nRetCode)
        {
            goto Exit1;
        }

        while (threadPool->m_TaskQueuesSpinlock.test_and_set());
        if (threadPool->m_vTaskQueue.empty())
        {
            threadPool->m_TaskQueuesSpinlock.clear();
            continue;
        }
        auto _callback = threadPool->m_vTaskQueue.front();
        threadPool->m_vTaskQueue.pop();

        threadPool->m_TaskQueuesSpinlock.clear();

        _callback();
    }
Exit1:
    return 0;
}

int JVBase::ThreadPool::InitWorkThreads()
{
    thread_ptr t;
    for (unsigned int i = 0; i < m_nThreadsNum; ++i)
    {
        t = thread_ptr(new std::thread(WorkThreadFuction, this));
        m_vptrThreads.push_back(t);
//        (*t).detach();
    }

    return 0;
}

void JVBase::ThreadPool::PushTask(callback_ptr fn)
{
    while (m_TaskQueuesSpinlock.test_and_set());

    m_vTaskQueue.push(fn);

    m_TaskQueuesSpinlock.clear();

    m_TaskSemaphore.Post();
}

void JVBase::ThreadPool::CreateThreadAndPushTask(callback_ptr fn)
{
    thread_ptr t = thread_ptr(new std::thread(WorkThreadFuction, this));

    while (m_TaskQueuesSpinlock.test_and_set());

    m_vptrThreads.push_back(t);
    m_nThreadsNum++;

    m_vTaskQueue.push(fn);

    m_TaskQueuesSpinlock.clear();

    m_TaskSemaphore.Post();
}