#pragma once

#include "tnet.h"

namespace tnet
{
    class IOEvent
    {
    public:
        IOEvent(int fd_, int events_, const IOHandler_t& handler_)
            : fd(fd_)
            , events(events_)
            , handler(handler_)
        {
            
        }
    
        int fd;
        int events;
        IOHandler_t handler;    
    };
}
