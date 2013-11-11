#include "ioloop.h"

#include <sys/epoll.h>
#include <stdlib.h>
#include <algorithm>

#include "poller.h"
#include "log.h"
#include "ioevent.h"

using namespace std;

namespace tnet
{
    const int DefaultEventsCapacity = 1024;
    const int MaxPollWaitTime = 1 * 1000;

    IOLoop::IOLoop()
    {
        m_running = false;     
    
        m_poller = new Poller(this); 
        
        m_events.resize(DefaultEventsCapacity, 0);
    }
    
    IOLoop::~IOLoop()
    {
        delete m_poller;
        
        for_each(m_events.begin(), m_events.end(), 
            default_delete<IOEvent>());
    }

    void IOLoop::start()
    {
        m_running = true;

        run();
    }

    void IOLoop::stop()
    {
        m_running = false;    
    }

    void IOLoop::run()
    {
        while(m_running)
        {
            m_poller->poll(MaxPollWaitTime, m_events);
        }
        
        LOG_INFO("loop stop");    
    }

    int IOLoop::addHandler(int fd, int events, const IOHandler_t& handler)
    {
        if(m_events.size() <= fd)
        {
            m_events.resize(fd, 0);    
        }

        if(m_events[fd] != 0)
        {
            LOG_ERROR("add duplicate handler %d", fd);
            return -1;    
        }

        if(m_poller->add(fd, events) != 0)
        {
            return -1;
        }

        m_events[fd] = new IOEvent(fd, events, handler);
        return 0;
    }

    int IOLoop::updateHandler(int fd, int events)
    {
        if(m_events.size() <= fd || m_events[fd] == 0)
        {
            LOG_ERROR("invalid fd %d", fd);
            return -1;    
        }

        if(m_events[fd]->events == events)
        {
            return 0;    
        }

        if(m_poller->update(fd, events) != 0)
        {
            return -1;
        }

        return 0;
    }

    int IOLoop::removeHandler(int fd)
    {
        if(m_events.size() <= fd || m_events[fd] == 0)
        {
            LOG_INFO("invalid fd %d", fd);
            return -1;    
        } 
        
        m_poller->remove(fd);

        delete m_events[fd];
        m_events[fd] = NULL;

        return 0;
    }

    void IOLoop::runAfter(int timeout, const Callback_t& callback)
    {
        
    }
}
