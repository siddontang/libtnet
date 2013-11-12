#include "connchecker.h"

#include "log.h"
#include "ioloop.h"
#include "timer.h"
#include "connection.h"

using namespace std;

namespace tnet
{
    const int DefaultInterval = 10 * 1000;
    const int DefaultStep = 1000;
    const int DefaultTimeout = 60;
    const int DefaultConnectTimeout = 20;

    ConnChecker::ConnChecker()
        : m_loop(0)
        , m_running(false)
        , m_step(DefaultStep)
        , m_lastPos(0)
        , m_timeout(DefaultTimeout)
        , m_connectTimeout(DefaultConnectTimeout)
    {
        m_timer = std::make_shared<Timer>(std::bind(&ConnChecker::onCheck, this, _1), DefaultInterval, 0); 
    }

    ConnChecker::~ConnChecker()
    {
        
    }

    void ConnChecker::start(IOLoop* loop)
    {
        if(m_running)
        {
            LOG_WARN("connchecker was started");    
        }     

        m_loop = loop;
        m_running = true;

        m_timer->start(loop);
    }

    void ConnChecker::stop()
    {
        if(!m_running)
        {
            LOG_WARN("connchecker was stopped");    
            return;
        }    

        m_timer->stop();
    }

    void ConnChecker::setInterval(int repeat)
    {
        m_timer->reset(repeat, 0);    
    }

    void ConnChecker::addConn(int fd, const WeakConnectionPtr_t& conn)
    {
        if(fd >= m_conns.size())
        {
            m_conns.resize(fd + 1);    
        }    

        WeakConnectionPtr_t old = m_conns[fd];
        if(old.lock())
        {
            LOG_ERROR("conn %d has exists, may be some error", fd);
            return;
        }

        LOG_INFO("%d %d", m_conns.size(), fd);
        m_conns[fd] = conn;
    }

    void ConnChecker::onCheck(const TimerPtr_t& timer)
    {
        LOG_INFO("check"); 
        
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        
        uint64_t now = t.tv_sec;
           
        size_t connNum = m_conns.size();

        for(size_t i = 0; i < m_step && i < connNum; ++i)
        {
            m_lastPos = (m_lastPos >= connNum) ? 0 : m_lastPos;
            
            ConnectionPtr_t conn = m_conns[i].lock();
            if(conn)
            {
                uint64_t duration  = now - conn->lastActiveTime();

                if(duration > m_timeout ||
                    (conn->isConnecting() && duration > m_connectTimeout))    
                {
                    conn->shutDown();    
                }
            }

            ++m_lastPos;
        }
    }
}
