#pragma once

#include <string>
#include <vector>

#include <initializer_list>

#include "address.h"
#include "tnet_redis.h"

namespace tnet
{
    class RedisClient : public nocopyable
                      , public std::enable_shared_from_this<RedisClient>
    {
    public:
        friend class RedisTrans;

        RedisClient(IOLoop* loop, const Address& addr, const std::string& password = std::string(), size_t maxClients = 10);
        ~RedisClient();

        void exec(std::initializer_list<std::string> cmd, const ReplyCallback_t& callback);
        void newTrans(const NewTransCallback_t& callback);

    private:
        void request(const std::string& data, const ReplyCallback_t& callback);

        RedisConnectionPtr_t popConn();
        void pushConn(const RedisConnectionPtr_t& conn);

        void onReply(const RedisConnectionPtr_t& conn, const RedisReply& reply, const ReplyCallback_t& callback);

        void onConnect(const RedisConnectionPtr_t& conn, int status, const std::vector<std::string>& cmd);

    private:
        IOLoop* m_loop;
        Address m_address;
        std::string m_password;
        size_t m_maxClients;

        std::vector<WeakRedisConnectionPtr_t> m_conns;

    };    
}
