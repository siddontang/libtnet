#include "tcpserver.h"

#include <algorithm>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>

#include "address.h"
#include "sockutil.h"
#include "log.h"
#include "acceptor.h"
#include "connection.h"
#include "ioloop.h"
#include "signaler.h"
#include "timer.h"
#include "process.h"
#include "connchecker.h"

using namespace std;

namespace tnet
{
    TcpServer::TcpServer()
        : m_loop(0)
    {
        m_process = std::make_shared<Process>();
    
        m_running = false;
    }
 
    TcpServer::~TcpServer()
    {
        if(m_loop)
        {
            delete m_loop;
        }
    }

    void TcpServer::initSignaler()
    {
        vector<int> signums;
        
        signums.push_back(SIGINT);
        signums.push_back(SIGTERM);

        m_signaler = std::make_shared<Signaler>(signums, std::bind(&TcpServer::onSignal, this, _1, _2)); 
    }

    int TcpServer::listen(const Address& addr, const ConnEventCallback_t& callback)
    {
        LOG_INFO("listen %s:%d", addr.ipstr().c_str(), addr.port());
        NewConnCallback_t cb = std::bind(&TcpServer::onNewConnection, this, _1, _2, callback);
        AcceptorPtr_t acceptor = std::make_shared<Acceptor>(cb);
        if(acceptor->listen(addr) < 0)
        {
            return -1;    
        }     

        m_acceptors.push_back(acceptor);
        return 0;
    }

    void TcpServer::run()
    {
        if(m_running)
        {
            return;    
        }

        m_loop = new IOLoop();
        
        m_running = true;
     
        initSignaler();
      
        m_connChecker = std::make_shared<ConnChecker>();
            
        for_each(m_acceptors.begin(), m_acceptors.end(), 
            std::bind(&Acceptor::start, _1, m_loop));
            
        m_connChecker->start(m_loop);

        m_signaler->start(m_loop);

        m_loop->start(); 
    }

    void TcpServer::start(size_t maxProcess)
    {
        if(maxProcess > 0)
        {
            m_process->wait(maxProcess, std::bind(&TcpServer::run, this));   
        }
        else
        {
            run();
        }
    }   

    void TcpServer::stop()
    {
        if(!m_running)
        {
            return;    
        }

        m_running = false;

        m_signaler->stop();
        
        for_each_all(m_acceptors, std::bind(&Acceptor::stop, _1));
        
        m_connChecker->stop();
         
        m_loop->stop();    
    }

    void TcpServer::onNewConnection(IOLoop* loop, int fd, const ConnEventCallback_t& callback)
    {
        LOG_INFO("new connection %d", fd); 
        ConnectionPtr_t conn = std::make_shared<Connection>(loop, fd);
        
        conn->setEventCallback(callback);

        conn->onEstablished();

        m_connChecker->addConn(fd, conn);

        return;
    }

    void TcpServer::onSignal(const SignalerPtr_t& signaler, int signum)
    {
        LOG_INFO("signum %d", signum);
        switch(signum)
        {
            case SIGINT:
            case SIGTERM:
                {
                    stop();
                }
                break;
            default:
                LOG_ERROR("invalid signal %d", signum);
                break;
        }    
    }
}
