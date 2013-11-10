#pragma once

#include <string>

#include "tnet.h"

namespace tnet
{
    class IOLoop;
    class Address;

    class Connection : public nocopyable
                     , public std::enable_shared_from_this<Connection>
    {
    public:
        friend class TcpServer;

        enum Status
        {
            None,
            Connecting,
            Connected,
            Disconnecting,
            Disconnected,    
        };
        
        Connection(IOLoop* loop, int fd);
        ~Connection();

        void setEventCallback(const ConnEventCallback_t& callback) { m_callback = callback; }

        //timeout is milliseconds, if timeout is 0, close immediately
        void close(int timeout = 0);
        void send(const std::string& data);

        void onEstablished();
        void connect(const Address& addr);

        uint64_t lastActiveTime(); 

    private:
        void onHandler(IOLoop*, int);
        void handleRead();
        void handleWrite();
        void handleError();
        void handleClose();
        void handleConnect();

        void updateActiveTime();

    private:
        ConnEventCallback_t m_callback;

        IOLoop* m_loop;
        int m_fd;
        int m_status;

        //seconds
        uint64_t m_lastActiveTime;

        std::string m_sendBuffer;
    };
    
}
