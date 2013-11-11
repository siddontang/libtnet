#include "timer.h"

#include <assert.h>
#include <sys/timerfd.h>
#include <errno.h>

#include "ioloop.h"
#include "log.h"

namespace tnet
{
    const uint64_t milliPerSeconds = 1000;
    const uint64_t microPerSeconds = 1000000;
    const uint64_t nanoPerSeconds = 1000000000;
    const uint64_t nanoPerMilli = 1000000;

    Timer::Timer(IOLoop* loop, const TimerHandler_t& handler, int repeat, int after)
        : m_loop(loop)
        , m_handler(handler)
    {
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if(m_fd < 0)
        {
            LOG_ERROR("create timer error %s", errorMsg(errno));
            return;
        } 

        reset(repeat, after);    
    }

    Timer::~Timer()
    {
        LOG_INFO("destroyed");
        if(m_fd > 0)
        {
            close(m_fd);
        }
    }

    void Timer::start()
    {
        assert(m_fd > 0);
        m_loop->addHandler(m_fd, TNET_READ, 
            std::bind(&Timer::onTimer, shared_from_this(), _1, _2));    
    }

    void Timer::stop()
    {
        assert(m_fd > 0);
        m_loop->removeHandler(m_fd);
    }

    void Timer::reset(int repeat, int after)
    {
        struct itimerspec t;
        if(repeat > 0)
        {
            t.it_interval.tv_sec = (uint64_t)repeat / milliPerSeconds;
            t.it_interval.tv_nsec = ((uint64_t)repeat % milliPerSeconds) * nanoPerMilli;
        }

        if(after > 0)
        {
            t.it_value.tv_sec = (uint64_t)after / milliPerSeconds;
            t.it_value.tv_nsec = ((uint64_t)after % milliPerSeconds) * nanoPerMilli; 
        }

        timerfd_settime(m_fd, 0, &t, NULL);
    }

    void Timer::onTimer(IOLoop* loop, int events)
    {
        TimerPtr_t timer = shared_from_this();
        
        m_handler(timer);     
    }
}
