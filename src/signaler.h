#pragma once

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Signaler : public nocopyable
    {
    public:
        Signaler(IOLoop* loop, int signum, const SignalHandler_t& handler);
        ~Signaler();

        void start();
        void stop();

    private:
        void onSignal(IOLoop*, int);

    private:
        IOLoop* m_loop;
        int m_signum;
        int m_fd;
        SignalHandler_t m_handler;
    };    
}
