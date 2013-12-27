#pragma once

#include "connection.h"
#include "log.h"
#include "sockutil.h"

namespace tnet
{
    template<typename Derived>
    Connector<Derived>::Connector()
    {
        
    }

    template<typename Derived>
    Connector<Derived>::~Connector()
    {
    }

    template<typename Derived>
    int Connector<Derived>::connect(IOLoop* loop, const Address& addr, const ConnectCallback_t& callback, const std::string& device)
    {
        int fd = SockUtil::create();
        if(fd < 0)
        {
            return fd;    
        } 
        
        if(!device.empty())
        {
            SockUtil::bindDevice(fd, device);
        }

        ConnectionPtr_t conn = std::make_shared<Connection>(loop, fd);
        
        m_conn = conn;
        
        conn->setEventCallback(std::bind(&Connector<Derived>::onConnConnectEvent, this->shared_from_this(), _1, _2, _3, callback));
        
        conn->connect(addr);
        
        return 0;    
    }

    template<typename Derived>
    void Connector<Derived>::onConnConnectEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context, const ConnectCallback_t& callback)
    {
        switch(event)
        {
            case Conn_ConnectingEvent:
                return;    
            case Conn_ConnectEvent:
                {
                    ConnectCallback_t cb = std::move(callback);

                    conn->setEventCallback(std::bind(&Connector<Derived>::onConnEvent, this->shared_from_this(), _1, _2, _3));
                
                    cb(this->shared_from_this(), true);
                }
                break;
            default:
                //here we think error
                callback(this->shared_from_this(), false);
                return;
                break;
        }    
    }

    template<typename Derived>
    void Connector<Derived>::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context)
    {
        DerivedPtr_t t = this->shared_from_this();
        switch(event)
        {
            case Conn_ReadEvent:
                {
                    const StackBuffer* buf = (const StackBuffer*)(context);
                    t->handleRead(buf->buffer, buf->count);
                }
                break; 
            case Conn_WriteCompleteEvent:
                t->handleWriteComplete(context);
                break;  
            case Conn_ErrorEvent:
                t->handleError(context);
                break;
            case Conn_CloseEvent:
                t->handleClose(context);
                break;
            default:
                break; 
        }    
    }

    template<typename Derived>
    void Connector<Derived>::send(const std::string& data)
    {
        ConnectionPtr_t conn = m_conn.lock();
        
        if(conn)
        {
            conn->send(data);    
        }    
    }

    template<typename Derived>
    void Connector<Derived>::shutDown()
    {    
        ConnectionPtr_t conn = m_conn.lock();
        
        if(conn)
        {
            conn->shutDown();    
        }
    }
}
