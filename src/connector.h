#pragma once

#include <string>

#include "tnet.h"

namespace tnet
{
    template<typename Derived>
    class Connector : public nocopyable
                    , public std::enable_shared_from_this<Derived>
    {
    public:
        Connector();
        Connector(const std::string& device);
        ~Connector();
        
        typedef std::shared_ptr<Derived> DerivedPtr_t;
        typedef std::function<void (const DerivedPtr_t& conn, bool connected)> ConnectCallback_t;
         
        int connect(IOLoop* loop, const Address& addr, const ConnectCallback_t& callback, const std::string& device = "");
       
        WeakConnectionPtr_t getConn() { return m_conn; } 
        ConnectionPtr_t lockConn() { return m_conn.lock(); }

        void send(const std::string& data);

        void shutDown();

    protected:
        void handleRead(const char*, size_t) {}
        void handleWriteComplete(const void*) {}
        void handleError(const void*) {}
        void handleClose(const void*) {}

    private:
        void onConnConnectEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context, const ConnectCallback_t& callback);
        
        void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context);

    private:
        WeakConnectionPtr_t m_conn;    
    };

}
