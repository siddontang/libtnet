#pragma once

#include <vector>

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Signaler : public nocopyable
                   , public std::enable_shared_from_this<Signaler>
    {
    public:
        Signaler(int signum, const SignalHandler_t& handler);
        Signaler(const std::vector<int>& signums, const SignalHandler_t& handler);
        ~Signaler();

        void start(IOLoop* loop);
        void stop();

    private:
        void resetFd(const std::vector<int>& signums);
        void onSignal(IOLoop*, int);

    private:
        IOLoop* m_loop;
        int m_fd;
        bool m_running;
        std::vector<int> m_signums;
        SignalHandler_t m_handler;
    };    
}
