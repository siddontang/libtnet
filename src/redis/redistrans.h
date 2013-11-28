#pragma once

#include <initializer_list>

#include "address.h"

#include "tnet_redis.h"

namespace tnet
{

    class RedisTrans : public nocopyable
                     , public std::enable_shared_from_this<RedisTrans>
    {
    public:
        friend class RedisClient;

        RedisTrans(const RedisClientPtr_t& client, const RedisConnectionPtr_t& conn);
        ~RedisTrans();

        void begin();
        
        void exec(std::initializer_list<std::string> cmd);

        void commit(const ReplyCallback_t& callback);
        void cancel(const ReplyCallback_t& callback); 

    private:
        void onReply(const RedisConnectionPtr_t& conn, const RedisReply& reply);
        void onConnect(const RedisConnectionPtr_t& conn, int status, const NewTransCallback_t& callback);

        void onCommit(const RedisReply& reply, const ReplyCallback_t& callback);
        void onCancel(const RedisReply& reply, const ReplyCallback_t& callback);

        void request(std::initializer_list<std::string> cmd);
    
    private:
        WeakRedisClientPtr_t m_client;

        WeakRedisConnectionPtr_t m_conn;
    
        ReplyCallback_t m_callback;

        int m_transNum;

        bool m_inTrans;
    };

}
