#include "connection.h"

#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "ioloop.h"
#include "log.h"
#include "sockutil.h"

using namespace std;

namespace tnet
{
    void dummyConnEvent(const ConnectionPtr_t&, ConnEvent, const void*)
    {
        
    }

    const int MaxReadBuffer = 4096;

    Connection::Connection(IOLoop* loop, int fd)
        : m_loop(loop)
        , m_fd(fd)
        , m_status(None)
    {
        m_callback = std::bind(&dummyConnEvent, _1, _2, _3);
    }

    Connection::~Connection()
    {
        LOG_INFO("connection destroyed %d", m_fd);
    }

    void Connection::clearEventCallback()
    {
        m_callback = std::bind(&dummyConnEvent, _1, _2, _3);
    }

    void Connection::updateActiveTime()
    {
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        
        m_lastActiveTime = t.tv_sec;        
    }

    void Connection::onEstablished()
    {
        if(m_status != None)
        {
            LOG_ERROR("invalid status %d, not None", m_status);
            return;    
        }
        
        m_status = Connected;
        
        updateActiveTime();
             
        ConnectionPtr_t conn = shared_from_this();
        m_loop->addHandler(m_fd, TNET_READ, std::bind(&Connection::onHandler, conn, _1, _2));
    
        m_callback(conn, Conn_EstablishedEvent, 0);
    }

    void Connection::connect(const Address& addr)
    {
        if(m_status != None)
        {
            LOG_ERROR("invalid status %d, must None", m_status);
            return;    
        } 

        ConnEvent event = Conn_ConnectEvent;
        int err = SockUtil::connect(m_fd, addr);
        if(err < 0)
        {
            if(err == EINPROGRESS)
            {
                m_status = Connecting;    
            }
            else
            {
                handleError();
                return;    
            }
        }
        else
        {
            m_status = Connected;    
        }

        updateActiveTime();
 
        ConnectionPtr_t conn = shared_from_this();

        m_loop->addHandler(m_fd, m_status == Connected ? TNET_READ : TNET_WRITE, 
            std::bind(&Connection::onHandler, conn, _1, _2));

        m_callback(conn, event, 0);
    }

    void Connection::onHandler(IOLoop* loop, int events)
    {
        //to prevent ref decr to 0
        ConnectionPtr_t conn = shared_from_this();
        
        if(events & TNET_READ)
        {
            handleRead();    
        }    
        
        if(events & TNET_WRITE)
        {
            if(m_status == Connecting)
            {
                handleConnect();   
            }
            else
            {
                handleWrite();    
            }
        }

        if(events & TNET_ERROR)
        {
            handleError();    
        }
    }

    void Connection::shutDown(int after)
    {
        if(m_status == Disconnecting || m_status == Disconnected)
        {
            return;    
        }

        m_status = Disconnecting;

        if(after == 0)
        {
            handleClose();    
        }    
        else
        {
            ConnectionPtr_t conn = shared_from_this();
            m_loop->runAfter(after, std::bind(&Connection::handleClose, conn));    
        }
    }

    void Connection::handleConnect()
    {
        if(m_status != Connected)
        {
            return;    
        }   
        
        if(SockUtil::getSockError(m_fd) != 0)
        {
            handleError();
            return;    
        }
        
        m_loop->updateHandler(m_fd, TNET_READ);

        updateActiveTime();

        m_status = Connected;    
        
        m_callback(shared_from_this(), Conn_ConnectEvent, 0);
    }
        
    void Connection::handleRead()
    {
        if(m_status != Connected)
        {
            return;    
        }

        char buf[MaxReadBuffer];
        int n = read(m_fd, buf, sizeof(buf));

        if(n > 0)
        {
            StackBuffer b(buf, n);

            updateActiveTime();
            
            m_callback(shared_from_this(), Conn_ReadEvent, &b);
            
            return;    
        }
        else if(n == 0)
        {
            handleClose();
            return;    
        }
        else
        {
            int err = errno;
            if(err == EAGAIN || err == EWOULDBLOCK)
            {
                //try write later, can enter here?
                LOG_INFO("read %s", errorMsg(err));
                return;    
            }    

            handleError();
            return;
        }
    }

    void Connection::handleWrite()
    {
        handleWrite(string());
    }

    void Connection::handleWrite(const string& data)
    {
        if(m_status != Connected)
        {
            return;    
        }    

        if(m_sendBuffer.empty() && data.empty())
        {
            m_loop->updateHandler(m_fd, TNET_READ);
            return;    
        }

        size_t totalSize = m_sendBuffer.size() + data.size();
        int niov = m_sendBuffer.empty() ? 1 : 2;
        int i = 0;

        struct iovec iov[niov];
        if(!m_sendBuffer.empty()) 
        {
            iov[i].iov_base = (void*)m_sendBuffer.data();
            iov[i].iov_len = m_sendBuffer.size();
            ++i;    
        }

        if(!data.empty())
        {
            iov[i].iov_base = (void*)data.data();
            iov[i].iov_len = data.size();
            ++i;    
        }

        ssize_t n = writev(m_fd, iov, niov);
        if(n == totalSize)
        {
            string().swap(m_sendBuffer);    
        
            m_callback(shared_from_this(), Conn_WriteCompleteEvent, 0);

            m_loop->updateHandler(m_fd, TNET_READ);

            updateActiveTime();

            return;
        }
        else if(n < 0)
        {
            int err = errno;
            if(err == EAGAIN || err == EWOULDBLOCK)
            {
                //try write later, can enter here?
                LOG_INFO("write %s", errorMsg(err));

                m_sendBuffer.append(data);

                return;   
            }    
            else
            {
                string().swap(m_sendBuffer);
                handleError();
                return;
            }
        }
        else
        {
            if(m_sendBuffer.size() < n)
            {
                n -= m_sendBuffer.size();

                m_sendBuffer = data.substr(n); 
            }
            else
            {
                m_sendBuffer = m_sendBuffer.substr(n);
                m_sendBuffer += data;    
            }
            
            updateActiveTime();
        }
    }

    bool Connection::disconnect()
    {
        if(m_status == Disconnected)
        {
            return false;    
        }    

        m_status = Disconnected;
        m_loop->removeHandler(m_fd);

        close(m_fd);

        return true;
    }

    void Connection::handleError()
    {
        disconnect();
        m_callback(shared_from_this(), Conn_ErrorEvent, 0);
    }

    void Connection::handleClose()
    {
        if(disconnect())
        {
            m_callback(shared_from_this(), Conn_CloseEvent, 0);
        }
    }

    int Connection::send(const string& data)
    {
        if(m_status != Connected)
        {
            LOG_INFO("send error");
            return -1;    
        }    
        
        handleWrite(data);
        return 0;
    }
}   

