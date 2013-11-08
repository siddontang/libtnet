#include "tcpserver.h"

#include "address.h"
#include "sockutil.h"
#include "log.h"

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
        NewConnCallback_t callback = std::bind(&TcpServer::onNewConnection, this, _1, callback);
        AcceptorPtr_t acceptor = AcceptorPtr_t(new Acceptor(m_loop, callback));
        if(acceptor->listen(addr) < 0)
        {
            return -1;    
        }     

        m_acceptors.push_back(acceptor);
        return 0;
    }

    int TcpServer::start(int maxProcess)
    {
        return 0;    
    }

    int TcpServer::stop()
    {
        
    }

    int TcpServer::onNewConnection(int fd, const ConnEventCallback_t& callback)
    {
        int curConns = Connection::getCurConnections();
        if(curConns > Connection::getMaxConnections())
        {
            LOG_ERROR("too many connections, exceed %d", Connection::getMaxConnections());
            close(fd);
            return -1;    
        }   
        
        ConnectionPtr_t conn = Connection::create(m_loop, fd);
        
        conn->setEventCallback(callback);

        return 0;
    }
}
