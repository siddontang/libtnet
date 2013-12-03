#include "redisconnection.h"

#include <vector>
#include <string>
#include <string.h>

#include "connection.h"
#include "log.h"
#include "sockutil.h"
#include "address.h"
#include "connector.inl"

using namespace std;

namespace tnet
{
    static void dummyCallback(const RedisConnectionPtr_t&, const RedisReply&)
    {
        
    }

    RedisConnection::RedisConnection()
        : Connector<RedisConnection>()
    {
        m_callback = std::bind(&dummyCallback, _1, _2); 
    
        m_context = redisContextInit();
    }

    RedisConnection::~RedisConnection()
    {
        redisFree(m_context);
        LOG_INFO("destroyed");     
    }

    void RedisConnection::handleRead(const char* buffer, size_t count)
    {
        ReplyCallback_t callback = bool(m_authCallback) ? m_authCallback : m_callback;

        struct redisReader* reader = m_context->reader;
        if(redisReaderFeed(reader, buffer, count) != REDIS_OK)
        {
            LOG_ERROR("redis read error %d %s", reader->err, reader->errstr);
       
            callback(shared_from_this(), RedisReply(reader->err));
        
            return;    
        }    

        do
        {
            struct redisReply* reply = NULL;
            if(redisGetReplyFromReader(m_context, (void**)&reply) == REDIS_ERR)
            {
                LOG_ERROR("redis get reply error %d %s", reader->err, reader->errstr);
                callback(shared_from_this(), RedisReply(reader->err));
                return;
            }

            if(reply != NULL)
            {
                callback(shared_from_this(), RedisReply(0, reply));
                freeReplyObject(reply);    
            }
            else
            {
                return;    
            }
        }while(true);

        return;
    }
    
    void RedisConnection::clearCallback()
    {
        m_callback = std::bind(&dummyCallback, _1, _2);    
    }
    
    string buildRequest(const vector<string>& args)
    {
        string str;
        str.reserve(1024);

        char buf[64];

        int n = snprintf(buf, sizeof(buf), "*%d\r\n", int(args.size()));

        str.append(buf, n);

        for(auto i = args.begin(); i != args.end(); ++i)
        {
            n = snprintf(buf, sizeof(buf), "$%d\r\n", int((*i).size()));
            str.append(buf);
            str.append(*i);
            str.append("\r\n");
        }

        return str;
    }

    void RedisConnection::exec(initializer_list<string> cmd)
    {
        return exec(vector<string>(cmd));
    }

    void RedisConnection::exec(const vector<string>& cmd)
    {
        send(buildRequest(cmd));
    }

    void RedisConnection::connect(IOLoop* loop, const Address& addr, const string& password, const ConnectCallback_t& callback)
    {
        Base_t::connect(loop, addr, std::bind(&RedisConnection::onConnect, shared_from_this(), _1, _2, password, callback));    
    }

    void RedisConnection::onConnect(const RedisConnectionPtr_t& conn, bool connected, const string& password, const ConnectCallback_t& callback)
    {
        if(!connected)
        {
            LOG_ERROR("redis connect error");
            callback(shared_from_this(), -1);
            return;    
        }    

        if(!password.empty())
        {
            string pass = std::move(password);
            ConnectCallback_t cb = std::move(callback);

            m_authCallback = std::bind(&RedisConnection::onAuth, shared_from_this(), _1, _2, cb);
            
            static const string AuthCmd = "AUTH";
            string data = buildRequest({AuthCmd, pass});

            conn->send(data);    
        }
        else
        {
            callback(shared_from_this(), 0);    
        }
    }

    void RedisConnection::onAuth(const RedisConnectionPtr_t& conn, const RedisReply& r, const ConnectCallback_t& callback)
    {  
        ConnectCallback_t cb = std::move(callback);

        ReplyCallback_t t; 
        m_authCallback.swap(t);

        struct redisReply* reply = r.reply;

        if(r.err != 0 || reply->type != REDIS_REPLY_STATUS && strcasecmp(reply->str, "OK") != 0)
        {
            cb(shared_from_this(), -2);
            conn->shutDown();
        }
        else
        {
            cb(shared_from_this(), 0);    
        }
        
    }
}
