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

using namespace std;

namespace tnet
{
    TcpServer::TcpServer()
    {
        m_loop = new IOLoop();
        m_running = false;
        m_workerProc = true;
        m_workerNum = 0;
        
        vector<int> signums;
        
        signums.push_back(SIGINT);
        signums.push_back(SIGTERM);
        signums.push_back(SIGCHLD);

        m_signaler = std::make_shared<Signaler>(m_loop, signums, std::bind(&TcpServer::onSignal, this, _1));
    }
 
    TcpServer::~TcpServer()
    {
        delete m_loop;
    }

    int TcpServer::listen(const Address& addr, const ConnEventCallback_t& callback)
    {
        LOG_INFO("listen %s:%d", addr.ipstr().c_str(), addr.port());
        NewConnCallback_t cb = std::bind(&TcpServer::onNewConnection, this, _1, _2, callback);
        AcceptorPtr_t acceptor = std::make_shared<Acceptor>(m_loop, cb);
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

        m_running = true;
        
        if(m_workerProc)
        {
            for_each(m_acceptors.begin(), m_acceptors.end(), 
                std::bind(&Acceptor::start, _1));
        }

        m_signaler->start();

        m_loop->start(); 
    }

    int TcpServer::startWorker(size_t workerNum)
    {
        for(size_t i = 0; i < workerNum; ++i)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG_ERROR("fork child error %s", errorMsg(errno));
                continue;    
            }
            else if(pid == 0)
            {
                LOG_INFO("start worker %d", getpid());
                m_workers.clear();
                m_workerProc = true;

                run();

                return 0;
            }
            else
            {
                m_workerProc = false;
                m_workers.insert(pid);            
            }
        }

        return 1;
    }

    void TcpServer::start(size_t maxProcess)
    {
        m_workerNum = maxProcess;
        m_workers.clear();

        if(maxProcess > 0)
        {
            if(startWorker(maxProcess) == 0)
            {
                //child return
                return;    
            }
        }

        LOG_INFO("run main process");

        run();
    }

    void TcpServer::stop()
    {
        if(!m_running)
        {
            return;    
        }

        m_running = false;

        if(m_workerProc)
        {
            for_each_all(m_acceptors, std::bind(&Acceptor::stop, _1));
        }

        if(m_workers.empty())
        {
            m_signaler->stop();

            m_loop->stop();    
        }
        else
        {
            assert(m_workerProc == false);   
       
            //notify work proc to quit
            for_each_all(m_workers, std::bind(kill, _1, SIGTERM));
        
            LOG_INFO("wait child to stop");
            TimerPtr_t timer = std::make_shared<Timer>(m_loop, std::bind(&TcpServer::onStopTimer, this, _1), 1000, 0);
            timer->start();
        }
    }

    void TcpServer::onStopTimer(const TimerPtr_t& timer)
    {
        LOG_INFO("on stop timer");
        checkWorkerDead();
        
        if(m_workers.empty())
        {
            timer->stop();

            m_signaler->stop();
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

    void TcpServer::checkWorkerDead()
    {
        pid_t pid;
        int status = 0;

        while((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            LOG_INFO("child was dead %d", pid);
            m_workers.erase(pid);
        }
    }   

    void TcpServer::handleSigChld()
    {
        LOG_INFO("handle Sig Child");
        assert(m_workerProc == false);
        
        checkWorkerDead();
        
        if(!m_running)
        {
            //main stop, now only wait child stop    
            if(m_workers.empty())
            {
                m_signaler->stop();
                
                m_loop->stop();    
            }
        }
        else
        {
            //main running, restart child
            LOG_INFO("worker was quit, restart it");
  
            size_t workerNum = m_workerNum - m_workers.size();
            
            if(workerNum > 0)
            {
                startWorker(workerNum);
            }
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
                    handleSigChld();                             
                }
                break;    
            default:
                LOG_ERROR("invalid signal %d", signum);
                break;
        }    
    }
}
