#pragma once

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Timer : public nocopyable
                , public std::enable_shared_from_this<Timer>
    {
    public:
        //repeat and after are all milliseconds
        Timer(const TimerHandler_t& handler, int repeat, int after);
        ~Timer();

        void start(IOLoop* loop);
        void stop();

        void reset(int repeat, int after);

        int fd() { return m_fd; }

    private:
        void onTimer(IOLoop* loop, int events);

        void initTimer(int repeat, int after);
    
    private:
        IOLoop* m_loop;
        int m_fd;
        bool m_running;
        
        TimerHandler_t m_handler;
    };
}
