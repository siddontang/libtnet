#pragma once

#include <vector>

#include "tnet.h"

extern "C"
{
struct epoll_event;    
}

namespace tnet
{
    class IOLoop;
    class IOEvent;

    //A wrapper for epoll 
    class Poller
    {
    public:
        Poller(IOLoop* loop);
        ~Poller();

        //timeout is milliseconds
        int poll(int timeout, const std::vector<IOEvent*>& events);

        int add(int fd, int events);
        int update(int fd, int events);
        int remove(int fd);

    private:
        IOLoop* m_loop;
        int m_fd;
    
        struct epoll_event* m_events;
        size_t m_eventSize;
    };
    
}
