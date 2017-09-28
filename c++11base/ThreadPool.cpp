
#include "ThreadPool.h"

using namespace std;
UnityGame::ThreadPool::ThreadPool()
{
    m_bRun = true;
    m_nThreadsNum = 1;
    this->InitWorkThreads();
}

UnityGame::ThreadPool::ThreadPool(unsigned int nThreadNum)
{
    m_bRun = true;
    m_nThreadsNum = nThreadNum;
    this->InitWorkThreads();
}

UnityGame::ThreadPool::~ThreadPool()
{
    if (m_bRun)
    {
        this->DestroyThreadPool();
    }
}
void UnityGame::ThreadPool::DestroyThreadPool()
{
    m_bRun = false;
    m_TaskSemaphore.DisableAndAwakeAll();
    for (unsigned int i = 0; i < m_nThreadsNum; ++i)
    {
        m_vptrThreads[i]->join();
        m_vptrThreads[i] = nullptr;
    }
    m_vptrThreads.clear();
}
static int UnityGame::WorkThreadFuction(UnityGame::ThreadPool* threadPool)
{
    int nRetCode;
    while (threadPool->m_bRun)
    {
        nRetCode = threadPool->m_TaskSemaphore.Wait();
        
        if (!threadPool->m_bRun || !nRetCode)
        {
            goto Exit1;
        }

        std::unique_lock<std::mutex> lck(threadPool->m_TaskQueueMutex);
        if (threadPool->m_vTaskQueue.empty())
        {
            lck.unlock();
            continue;
        }
        auto _callback = threadPool->m_vTaskQueue.front();
        threadPool->m_vTaskQueue.pop();
        lck.unlock();

        _callback();
    }
Exit1:
    return 0;
}

int UnityGame::ThreadPool::InitWorkThreads()
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

void UnityGame::ThreadPool::PushTask(callback_ptr fn)
{
    std::unique_lock<std::mutex> lck(m_TaskQueueMutex);
    m_vTaskQueue.push(fn);
    lck.unlock();
    m_TaskSemaphore.Post();
}

void UnityGame::ThreadPool::CreateThreadAndPushTask(callback_ptr fn)
{
    std::unique_lock<std::mutex> lck(m_TaskQueueMutex);

    thread_ptr t = thread_ptr(new std::thread(WorkThreadFuction, this));
    m_vptrThreads.push_back(t);
    m_nThreadsNum++;

    m_vTaskQueue.push(fn);
    lck.unlock();

    m_TaskSemaphore.Post();
}