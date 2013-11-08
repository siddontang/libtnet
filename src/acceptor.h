#pragma once

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Address;

    class Acceptor : public nocopyable
    {
    public:
        Acceptor(IOLoop* loop, const NewConnCallback_t& callback);
        ~Acceptor();
    
        int listen(const Address& addr);

        void start();
        void stop();

    private:
        void onAccept(IOLoop*, int);

    private:
        IOLoop* m_loop;
        
        int m_sockFd;
        int m_dummyFd;    
    
        NewConnCallback_t m_callback; 
    };   
}
