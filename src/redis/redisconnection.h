#pragma once

#include <initializer_list>

#include "tnet_redis.h"
#include "connector.h"

namespace tnet
{
    class Address;
    class RedisConnection : public Connector<RedisConnection>
    {
    public:
        typedef std::function<void (const RedisConnectionPtr_t&, const RedisReply&)> ReplyCallback_t;

        typedef Connector<RedisConnection> Base_t;
        friend class Connector<RedisConnection>;

        RedisConnection();
        ~RedisConnection();
        
        void clearCallback();
        void setCallback(const ReplyCallback_t& callback) { m_callback = callback; }

        void exec(std::initializer_list<std::string> cmd);
        void exec(const std::vector<std::string>& cmd);

        //ConnectCallback status: 0 for ok, -1 for not connected, -2 for auth fail
        typedef std::function<void (const RedisConnectionPtr_t& conn, int status)> ConnectCallback_t;   
        void connect(IOLoop* loop, const Address& addr, const std::string& password, const ConnectCallback_t& callback);

    private:
        void handleRead(const char*, size_t);

        void onConnect(const RedisConnectionPtr_t& conn, bool connected, const std::string& password, const ConnectCallback_t& callback);
        void onAuth(const RedisConnectionPtr_t& conn, const RedisReply& reply, const ConnectCallback_t& callback);

    private:
        ReplyCallback_t m_callback;

        ReplyCallback_t m_authCallback;

        struct redisContext* m_context; 
    };       
}
