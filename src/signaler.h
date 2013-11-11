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
        Signaler(IOLoop* loop, int signum, const SignalHandler_t& handler);
        Signaler(IOLoop* loop, const std::vector<int>& signums, const SignalHandler_t& handler);
        ~Signaler();

        void start();
        void stop();

    private:
        void resetFd(const std::vector<int>& signums);
        void onSignal(IOLoop*, int);

    private:
        IOLoop* m_loop;
        int m_fd;
        std::vector<int> m_signums;
        SignalHandler_t m_handler;
    };    
}
