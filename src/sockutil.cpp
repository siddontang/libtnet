#include "sockutil.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <assert.h>

#include "address.h"
#include "log.h"

namespace tnet
{
    int SockUtil::create()
    {
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);    
        if(fd < 0)
        {
            return fd;    
        }

        return fd;
    }

    int SockUtil::bindAndListen(const Address& addr)
    {
        int err = 0;
        
        int fd = create();
        if(fd < 0)
        {
            err = errno;
            LOG_ERROR("create socket error %s", errorMsg(err));
            return fd;    
        }
            
        SockUtil::setReuseable(fd, true);
        
        do
        {
            struct sockaddr_in sockAddr = addr.sockAddr();
            
            if(bind(fd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0)
            {
                err = errno;
                LOG_ERROR("bind address %s:%d error: %s", addr.ipstr().c_str(), addr.port(), errorMsg(err));
                break;    
            }

            if(listen(fd, SOMAXCONN) < 0)
            {
                err = errno;
                LOG_ERROR("listen address %s:%d error: %s", addr.ipstr().c_str(), addr.port(), errorMsg(err));
                break;     
            }


            return fd;

        }while(0);

        close(fd);
        return err;
    }

    int SockUtil::connect(int sockFd, const Address& addr)
    {
        struct sockaddr_in sockAddr = addr.sockAddr();
    
        int ret = ::connect(sockFd, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
        if(ret < 0)
        {
            int err = errno;
            return err;    
        }
        else
        {
            SockUtil::setNoDelay(sockFd, true);
            return ret;
        }
    }

    int SockUtil::setNoDelay(int sockFd, bool on)
    {
        int opt = on ? 1 : 0;
        
        return setsockopt(sockFd, IPPROTO_TCP, 
                        TCP_NODELAY, &opt, 
                        static_cast<socklen_t>(sizeof(opt)));    
    }
    
    int SockUtil::setReuseable(int sockFd, bool on)
    {
        int opt = on ? 1 : 0;
        return setsockopt(sockFd, SOL_SOCKET, 
                        SO_REUSEADDR, &opt,
                        static_cast<socklen_t>(sizeof(opt)));    
    }

    int SockUtil::setKeepAlive(int sockFd, bool on)
    {
        int opt = on ? 1 : 0;
        return setsockopt(sockFd, SOL_SOCKET, 
                        SO_KEEPALIVE, &opt,
                        static_cast<socklen_t>(sizeof(opt)));    
    }

    int SockUtil::getLocalAddr(int sockFd, Address& addr)
    {
        struct sockaddr_in sockAddr;
        socklen_t sockLen = sizeof(sockAddr);
        if(getsockname(sockFd, (struct sockaddr*)&sockAddr, &sockLen) != 0)
        {
            int err = errno;
            return err;
        }

        addr = Address(sockAddr);
        return 0;
    }

    int SockUtil::getRemoteAddr(int sockFd, Address& addr)
    {
        struct sockaddr_in sockAddr;
        socklen_t sockLen = sizeof(sockAddr);
        if(getpeername(sockFd, (struct sockaddr*)&sockAddr, &sockLen) != 0)
        {
            int err = errno;
            return err;
        }

        addr = Address(sockAddr);
        return 0;
    }

    int SockUtil::getSockError(int sockFd)
    {
        int opt;
        socklen_t optLen = static_cast<socklen_t>(sizeof(opt));
        
        if(getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &opt, &optLen) < 0)
        {
            int err = errno;
            return err;    
        }   
        else
        {
            return opt;    
        }
    }

}
