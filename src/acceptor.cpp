#include "acceptor.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>

#include <memory>
#include <functional>

#include "ioloop.h"
#include "address.h"
#include "sockutil.h"
#include "log.h"

namespace tnet
{
    int createDummyFd()
    {
        return open("/dev/null", O_RDONLY | O_CLOEXEC);    
    }

    Acceptor::Acceptor(const NewConnCallback_t& callback)
        : m_loop(0)
        , m_sockFd(0)
        , m_dummyFd(createDummyFd())
        , m_running(false)
        , m_callback(callback)
    {
        
    }

    Acceptor::~Acceptor()
    {
        if(m_sockFd > 0)
        {
            close(m_sockFd);
        }
        close(m_dummyFd);    
    }

    int Acceptor::listen(const Address& addr)
    {
        int fd = SockUtil::bindAndListen(addr);
        if(fd < 0)
        {
            return fd;    
        }        

        m_sockFd = fd;
    
        return m_sockFd;
    }

    void Acceptor::start(IOLoop* loop)
    {
        assert(m_sockFd > 0);

        if(m_running)
        {
            LOG_WARN("acceptor was started");
            return;    
        }

        m_loop = loop;
    
        m_running = true;
        
        m_loop->addHandler(m_sockFd, TNET_READ,
            std::bind(&Acceptor::onAccept, this, _1, _2));  
    }

    void Acceptor::stop()
    {
        assert(m_sockFd > 0);
        if(!m_running)
        {
            LOG_WARN("acceptor was stoped");
            return;    
        }

        m_running = false;

        LOG_INFO("stop %d", m_sockFd);
        m_loop->removeHandler(m_sockFd);    
    }

    void Acceptor::onAccept(IOLoop* loop, int events)
    {
        int sockFd = accept4(m_sockFd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(sockFd < 0)
        {
            LOG_INFO("accept error");
            int err = errno;
            if(err == EMFILE || err == ENFILE)
            {
                close(m_dummyFd);
                sockFd = accept(m_sockFd, NULL, NULL);
                close(sockFd);
                
                m_dummyFd = createDummyFd();    
            }    
            return;
        }
        else
        {
            LOG_INFO("onAccept %d", sockFd);
            
            SockUtil::setNoDelay(sockFd, true);

            m_callback(loop, sockFd);    
        } 
    }
}
