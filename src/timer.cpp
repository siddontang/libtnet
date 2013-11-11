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

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        t.it_value.tv_sec = now.tv_sec + (uint64_t)after / milliPerSeconds;
        t.it_value.tv_nsec = now.tv_nsec + ((uint64_t)after % milliPerSeconds) * nanoPerMilli; 

        if(timerfd_settime(m_fd, TFD_TIMER_ABSTIME, &t, NULL) < 0)
        {
            LOG_ERROR("set timer error");    
        } 
    }

    void Timer::onTimer(IOLoop* loop, int events)
    {
        TimerPtr_t timer = shared_from_this();

        uint64_t exp;
        ssize_t s = read(m_fd, &exp, sizeof(uint64_t));
        if(s != sizeof(exp))
        {
            LOG_ERROR("onTimer read error");       
        }
        else
        {
            m_handler(timer);     
        }
    }
}
