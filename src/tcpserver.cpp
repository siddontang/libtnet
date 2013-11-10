#include "tcpserver.h"

#include <algorithm>

#include "address.h"
#include "sockutil.h"
#include "log.h"
#include "acceptor.h"
#include "connection.h"
#include "ioloop.h"

using namespace std;

namespace tnet
{
    TcpServer::TcpServer()
    {
        m_loop = new IOLoop();
    }
 
    TcpServer::~TcpServer()
    {
        delete m_loop;
    }

    int TcpServer::listen(const Address& addr, const ConnEventCallback_t& callback)
    {
        LOG_INFO("listen %s:%d", addr.ipstr().c_str(), addr.port());
        NewConnCallback_t cb = std::bind(&TcpServer::onNewConnection, this, _1, _2, callback);
        AcceptorPtr_t acceptor = AcceptorPtr_t(new Acceptor(m_loop, cb));
        if(acceptor->listen(addr) < 0)
        {
            return -1;    
        }     

        m_acceptors.push_back(acceptor);
        return 0;
    }

    void TcpServer::start(int maxProcess)
    {
        for_each(m_acceptors.begin(), m_acceptors.end(), 
            std::bind(&Acceptor::start, _1));

        m_loop->start();
    }

    void TcpServer::stop()
    {
        m_loop->stop();
        for_each(m_acceptors.begin(), m_acceptors.end(), 
            std::bind(&Acceptor::stop, _1));
    }

    void TcpServer::onNewConnection(IOLoop* loop, int fd, const ConnEventCallback_t& callback)
    {
        LOG_INFO("new connection %d", fd); 
        ConnectionPtr_t conn = std::make_shared<Connection>(loop, fd);
        
        conn->setEventCallback(callback);

        conn->onEstablished();

        return;
    }
}
