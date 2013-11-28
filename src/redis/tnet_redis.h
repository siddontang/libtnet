#pragma once

#include <vector>
#include <string>

#include "tnet.h"

extern "C"
{
#include "hiredis.h"
}

namespace tnet
{
    class RedisClient;
    class RedisReply;
    class RedisConnection;
    class RedisTrans;

    typedef std::shared_ptr<RedisClient> RedisClientPtr_t;
    typedef std::weak_ptr<RedisClient> WeakRedisClientPtr_t;
    typedef std::shared_ptr<RedisConnection> RedisConnectionPtr_t;
    typedef std::weak_ptr<RedisConnection> WeakRedisConnectionPtr_t;
    typedef std::shared_ptr<RedisTrans> RedisTransPtr_t;

    class RedisReply
    {
    public:
        RedisReply() 
        {
            err = 0;
            reply = NULL;
        }

        RedisReply(int e, struct redisReply* r = 0)
        {
            err = e;
            reply = r;
        }

        int err;  //0 for ok
        struct redisReply* reply;
    };

    typedef std::function<void (const RedisReply&)> ReplyCallback_t;

    typedef std::function<void (const RedisTransPtr_t&, int)> NewTransCallback_t;
}
