#include "poller.h"

#include <sys/types.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "log.h"
#include "ioevent.h"

using namespace std;

namespace tnet
{
    const int DefaultEventSize = 1024;
    const int MaxEventSize = 10240;

    Poller::Poller(IOLoop* loop)
        : m_loop(loop)
    {
        m_fd = epoll_create1(EPOLL_CLOEXEC);
        if(m_fd < 0)
        {
            LOG_ERROR("epoll create error %s", errorMsg(errno));    
        } 

        m_eventSize = DefaultEventSize;
        m_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * m_eventSize); 
    }

    Poller::~Poller()
    {
        if(m_fd > 0)
        {
            close(m_fd);    
        }    

        free(m_events);
    }

    int Poller::add(int fd, int events)
    {
        assert(m_fd > 0);
        struct epoll_event event;
     
        event.data.u64 = 0;
        
        event.data.fd = fd;
        event.events = (events & TNET_READ ? EPOLLIN : 0)
                        | (events & TNET_WRITE ? EPOLLOUT : 0);   

        int ret = epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event);
        if(ret < 0)
        {
            LOG_ERROR("epoll_ctl add error %s", errorMsg(errno));
            return -1;
        }

        return 0;
    }

    int Poller::update(int fd, int events)
    {
        assert(m_fd > 0);

        struct epoll_event event;
        
        event.data.u64 = 0;
        event.data.fd = fd;

        event.events = (events & TNET_READ ? EPOLLIN : 0)
                     | (events & TNET_WRITE ? EPOLLOUT : 0);   

        int ret = epoll_ctl(m_fd, EPOLL_CTL_MOD, fd, &event);
        if(ret < 0)
        {
            LOG_ERROR("epoll_ctl update error %s", errorMsg(errno));
            return -1;    
        }

        return 0;
    }

    int Poller::remove(int fd)
    {
        assert(m_fd > 0);    

        int ret = epoll_ctl(m_fd ,EPOLL_CTL_DEL, fd, 0);
        if(ret < 0)
        {
            LOG_ERROR("epoll_ctl remove error %s", errorMsg(errno));
            return -1;    
        }

        return 0;
    }

    int Poller::poll(int timeout, const std::vector<IOEvent*>& events)
    {
        memset(m_events, 0, sizeof(struct epoll_event) * m_eventSize);

        int ret = epoll_wait(m_fd, m_events, m_eventSize, timeout);
        if(ret < 0)
        {
            LOG_ERROR("epoll_wait error %s", errorMsg(errno));
            return -1;    
        }

        for(int i = 0; i < ret; ++i)
        {
            struct epoll_event* ev = m_events + i;
            int fd = ev->data.fd;
            int got = (ev->events & (EPOLLOUT | EPOLLERR | EPOLLHUP) ? TNET_WRITE : 0)
                    | (ev->events & (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP) ? TNET_READ : 0);

            IOEvent* io = fd < events.size() ? events[fd] : 0;
            
            if(!io)
            {
                //may occur event cache problem, see man 7 epoll
                continue;
            }
            
            //int want = io ? io->events : TNET_NONE;
            int want = io->events;

            if(got & ~want)
            {
                ev->events = (want & TNET_READ ? EPOLLIN : 0)
                           | (want & TNET_WRITE ? EPOLLOUT : 0);
                if(epoll_ctl(m_fd, want ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, ev) < 0)
                { 
                    LOG_ERROR("ctl error %s got:%d, want:%d, fd:%d", errorMsg(errno), got, want, fd);
                    continue;
                }
            }

            (io->handler)(m_loop, got);
        }

        if(ret == m_eventSize && m_eventSize != MaxEventSize)
        {
            m_eventSize *= 2;
            if(m_eventSize > MaxEventSize)
            {
                m_eventSize = MaxEventSize;    
            }            

            m_events = (struct epoll_event*)realloc(m_events, sizeof(struct epoll_event) * m_eventSize);
        }

        return ret;
    }
}
