#pragma once

#include <vector>

#include "tnet.h"

namespace tnet
{
    class Address;
    class Acceptor;
    class IOLoop;

    class TcpServer : public nocopyable
    {
    public:
        TcpServer();
        ~TcpServer();
        
        int listen(const Address& addr, const ConnEventCallback_t& callback);
    
        void start(int maxProcess);
        void stop();
        
    private:
        void onNewConnection(int fd, const ConnEventCallback_t& callback);

    private:
        IOLoop* m_loop;

        std::vector<AcceptorPtr_t> m_acceptors;
    };
     
}
