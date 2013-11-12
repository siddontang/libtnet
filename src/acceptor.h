#pragma once

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Address;

    class Acceptor : public nocopyable
    {
    public:
        Acceptor(const NewConnCallback_t& callback);
        ~Acceptor();
    
        int listen(const Address& addr);

        void start(IOLoop* loop);
        void stop();

    private:
        void onAccept(IOLoop*, int);

    private:
        IOLoop* m_loop;
        
        int m_sockFd;
        int m_dummyFd;    
   
        bool m_running;
    
        NewConnCallback_t m_callback; 
    };   
}
