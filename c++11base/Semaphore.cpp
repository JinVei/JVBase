
#include "Semaphore.h"

JVBase::Semaphore::Semaphore()
{
    m_Available = true;
    m_AvailableResource = 0;
}

JVBase::Semaphore::Semaphore(long ResourceNumber)
{
    m_Available = true;

    if (ResourceNumber >= 0) {
        m_AvailableResource = ResourceNumber;
    } else {
        m_AvailableResource = 0;
    }
}

JVBase::Semaphore::~Semaphore()
{
    DisableAndAwakeAll();
}

static bool JVBase::CheckCondition(JVBase::Semaphore* sem)
{
    return sem->m_AvailableResource > 0 || !sem->m_Available;
}

void JVBase::Semaphore::WaitResource()
{
    std::unique_lock <std::mutex> lck(m_MutexLock);

    m_ConditionLock.wait(lck, std::bind(CheckCondition, this));

    --m_AvailableResource;
    lck.unlock();
}

void JVBase::Semaphore::DisableAndAwakeAll()
{
    m_Available = false;
    m_ConditionLock.notify_all();
}

bool JVBase::Semaphore::Wait()
{
    if (!m_Available) {
        return false;
    }

    WaitResource();
//    m_ConditionLock.notify_one();

    return m_Available ? true : false;
}

bool  JVBase::Semaphore::Post()
{
    if (!m_Available) {
        return false;
    }
    else
    {
        std::unique_lock <std::mutex> lck(m_MutexLock);
        ++m_AvailableResource;
        lck.unlock();
        m_ConditionLock.notify_one();
        return true;
    }
}
