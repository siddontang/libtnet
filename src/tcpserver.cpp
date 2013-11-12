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

using namespace std;

namespace tnet
{
    TcpServer::TcpServer()
        : m_loop(0)
    {
        m_process = std::make_shared<Process>();
    
        m_running = false;

        initSignaler();
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
        signums.push_back(SIGCHLD);

        m_signaler = std::make_shared<Signaler>(signums, std::bind(&TcpServer::onSignal, this, _1)); 
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
      
        if(!m_process->hasChild())
        {  
            for_each(m_acceptors.begin(), m_acceptors.end(), 
                std::bind(&Acceptor::start, _1, m_loop));
        }

        m_signaler->start(m_loop);

        m_loop->start(); 
    }

    void TcpServer::start(size_t maxProcess)
    {
        if(maxProcess > 1)
        {
            m_process->start(maxProcess);   
        }

        LOG_INFO("run process %d, ismain %d", getpid(), m_process->isMainProc());

        run();
    }

    void TcpServer::stop()
    {
        if(!m_running)
        {
            return;    
        }

        m_running = false;

        m_signaler->stop();
        
        if(!m_process->hasChild()) 
        {
            for_each_all(m_acceptors, std::bind(&Acceptor::stop, _1));
         
            m_loop->stop();    
        }
        else
        {
            assert(m_process->isMainProc());
            
            m_process->stop();
        
            LOG_INFO("wait child to stop");
            TimerPtr_t timer = std::make_shared<Timer>(std::bind(&TcpServer::onStopTimer, this, _1), 1000, 0);
            timer->start(m_loop);
        }
    }

    void TcpServer::onStopTimer(const TimerPtr_t& timer)
    {
        LOG_INFO("on stop timer");
       
        m_process->wait();
        
        if(!m_process->hasChild())
        {
            timer->stop();

            m_loop->stop();    
        }
    }

    void TcpServer::onNewConnection(IOLoop* loop, int fd, const ConnEventCallback_t& callback)
    {
        LOG_INFO("new connection %d", fd); 
        ConnectionPtr_t conn = std::make_shared<Connection>(loop, fd);
        
        conn->setEventCallback(callback);

        conn->onEstablished();

        return;
    }

    void TcpServer::onSubProcDead()
    {
        LOG_INFO("onSubProcDead");
        assert(m_process->isMainProc());
        
        int deads = m_process->wait();
        
        if(!m_running)
        {
            //main stop, now only wait child stop    
            if(!m_process->hasChild())
            {
                m_signaler->stop();
                
                m_loop->stop();    
            }
        }
        else
        {
            //main running, restart child
            LOG_INFO("worker was quit, restart it");
            m_process->restart(); 
        } 
    }

    void TcpServer::onSignal(int signum)
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
            case SIGCHLD:
                {
                    onSubProcDead();
                }
                break;    
            default:
                LOG_ERROR("invalid signal %d", signum);
                break;
        }    
    }
}
