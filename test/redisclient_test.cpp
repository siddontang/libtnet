#include <iostream>

#include <assert.h>

#include "redisclient.h"
#include "ioloop.h"
#include "log.h"
#include "address.h"
#include "redistrans.h"

using namespace std;
using namespace tnet;

void getCallback(const RedisClientPtr_t& client, const RedisReply& reply)
{
    cout << "getCallback" << endl;
    if(reply.err != 0)
    {
        cout << "redis error %d" << reply.err << endl;    
    }
    else
    {
        cout << reply.reply->str << endl;    
    }
}

void setCallback(const RedisClientPtr_t& client, const RedisReply& reply)
{
    if(reply.err != 0)
    {
        cout << "redis error %d" << reply.err << endl;    
    }
    else
    {
        client->exec({"GET", "key"}, std::bind(&getCallback, client, _1));  
    }
}

void onCommit(const RedisReply& r)
{
    if(r.err != 0)
    {
        cout << "commit error " << r.err << endl;
        return;    
    }

    struct redisReply* reply = r.reply;
    if(reply->type != REDIS_REPLY_ARRAY)
    {
        cout << "multi error" << endl;    
    }

    cout << reply->elements << endl;

    for(size_t i = 0; i < reply->elements; ++i)
    {
        struct redisReply* elem = reply->element[i];
        cout << elem->str << endl;    
    }
}

void onNewTrans(const RedisTransPtr_t& trans, int status)
{
    if(status != 0)
    {
        cout << "new trans error\t" << status << endl;    
        return;
    }    

    trans->begin();

    trans->exec({"SET", "KEY1", "VALUE1"});
    trans->exec({"SET", "KEY2", "VALUE2"});
    trans->exec({"SET", "KEY3", "VALUE3"});
    trans->exec({"GET", "KEY1"});
    trans->exec({"GET", "KEY2"});
    trans->exec({"GET", "KEY3"});

    trans->commit(std::bind(&onCommit, _1));
}

int main()
{
    IOLoop loop;

    Address addr("127.0.0.1", 6379);
    RedisClientPtr_t client = std::make_shared<RedisClient>(&loop, addr);

    client->exec({"SET", "key", "hello world"}, std::bind(&setCallback, client, _1));  

    client->newTrans(std::bind(&onNewTrans, _1, _2));

    loop.start();

    return 0;    
}
