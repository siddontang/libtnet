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
    
    class TcpServer : public nocopyable
    {
    public:
        TcpServer();
        ~TcpServer();
        
        int listen(const Address& addr, const ConnEventCallback_t& callback);
   
        void start(size_t maxProcess = 0);
        void stop();
        
        IOLoop* getLoop() { return m_loop; }

    private:
        void initSignaler();
        
        void run();
        void onSignal(int signum);
        void onNewConnection(IOLoop* loop, int fd, const ConnEventCallback_t& callback);

        void onSubProcDead();

        void onStopTimer(const TimerPtr_t& timer);

    private:
        IOLoop* m_loop;
        
        ProcessPtr_t m_process;

        std::vector<AcceptorPtr_t> m_acceptors;

        SignalerPtr_t m_signaler;

        std::set<pid_t> m_workers;
        size_t m_workerNum;

        bool m_workerProc;
        bool m_running;
    };
     
}
