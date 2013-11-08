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

    Acceptor::Acceptor(IOLoop* loop, const NewConnCallback_t& callback)
        : m_loop(loop)
        , m_sockFd(0)
        , m_dummyFd(createDummyFd())
        , m_callback(callback)
    {
        
    }

    Acceptor::~Acceptor()
    {
        close(m_sockFd);
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

    void Acceptor::start()
    {
        if(m_sockFd < 0)
        {
            LOG_ERROR("invalid sock fd, can't start");
            return;    
        } 
    }

    void Acceptor::stop()
    {
        assert(m_sockFd > 0);
        m_loop->removeHandler(m_sockFd);    
    }

    void Acceptor::onAccept(IOLoop* loop, int events)
    {
        int sockFd = accept4(m_sockFd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(sockFd < 0)
        {
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
            m_callback(loop, sockFd);    
        } 
    }
}
