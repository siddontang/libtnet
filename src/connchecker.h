#pragma once

#include <vector>

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    
    class ConnChecker
    {
    public:
        ConnChecker();
        ~ConnChecker();

        void setRepeat(int repeat);
        void setStep(size_t step) { m_step = step; }
        
        //timeout are seconds
        void setTimeout(uint64_t timeout) { m_timeout = timeout; }
        void setConnectTimeout(uint64_t timeout) { m_connectTimeout = timeout; }

        void start(IOLoop* loop);
        void stop();
   
        void addConn(int fd, const WeakConnectionPtr_t& conn);
   
    private:
        void onCheck(const TimerPtr_t& timer);
    
    private:
        IOLoop* m_loop;
        bool m_running;
    
        size_t m_step;
        size_t m_lastPos;
   
        uint64_t m_timeout;
        uint64_t m_connectTimeout;
        
        std::vector<WeakConnectionPtr_t> m_conns;
  
        TimerPtr_t m_timer;
    };
    
}
