#pragma once

#include <vector>
#include <set>
#include <unistd.h>

#include "tnet.h"

namespace tnet
{
    class Address;
    class IOLoop;
    class Process;
    class ConnChecker;
    
    class TcpServer : public nocopyable
    {
    public:
        TcpServer();
        ~TcpServer();
        
        int listen(const Address& addr, const ConnEventCallback_t& callback);
   
        void start(size_t maxProcess = 0);
        void stop();
        
        IOLoop* getLoop() { return m_loop; }

        void setRunCallback(const ServerRunCallback_t& callback) { m_runCallback = callback; }

        //timeout is second
        void setMaxIdleTimeout(int timeout) { m_maxIdleTimeout = timeout; }

    private:
        void run();
        void onRun();
        void onStop();
        void onSignal(const SignalerPtr_t& signaler, int signum);
        void onIdleConnCheck(const TimingWheelPtr_t& wheel, const WeakConnectionPtr_t& conn);
        void onNewConnection(IOLoop* loop, int fd, const ConnEventCallback_t& callback);

    private:
        IOLoop* m_loop;
        
        ProcessPtr_t m_process;

        std::vector<AcceptorPtr_t> m_acceptors;

        SignalerPtr_t m_signaler;

        int m_maxIdleTimeout;
        TimingWheelPtr_t m_idleWheel;

        std::set<pid_t> m_workers;
        size_t m_workerNum;

        bool m_workerProc;
        bool m_running;
   
        ServerRunCallback_t m_runCallback;
    };
     
}
