#pragma once

#include <vector>

#include "tnet.h"

namespace tnet
{
    class IOEvent;
    class Poller;

    class IOLoop
    {
    public:
        IOLoop();
        ~IOLoop();

        void start();
        void stop();

        int addHandler(int fd, int events, const IOHandler_t& handler);
        int updateHandler(int fd, int events);
        int removeHandler(int fd);  

        void runAfter(int timeout, const Callback_t& callback);

    private:
        void run();

    private:
        int m_pollFd;        
    
        bool m_running;
    
        std::vector<IOEvent*> m_events;

        Poller* m_poller;
    };
    
}
