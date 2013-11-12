#pragma once

#include "tnet.h"

namespace tnet
{
    class SpinLock : public nocopyable
    {
    public:
        SpinLock() {
            m_lock = 0;    
        }
        ~SpinLock() {} 

        void lock()
        {
            while(__sync_lock_test_and_set(&m_lock, 1))
            {
            }    
        }

        void unlock()
        {
            __sync_lock_release(&m_lock);    
        }
        
    private:
        volatile int m_lock;   
    }; 

    class SpinLockGuard : public nocopyable
    {
    public:
        SpinLockGuard(SpinLock& lock) 
            : m_lock(lock)
        {
            m_lock.lock();   
        }

        ~SpinLockGuard()
        {
            m_lock.unlock();
        }
    private:
        SpinLock& m_lock;
    };
}
