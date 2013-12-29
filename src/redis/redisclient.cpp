#include "redisclient.h"

#include <string.h>

#include "ioloop.h"
#include "connection.h"
#include "log.h"
#include "address.h"
#include "sockutil.h"
#include "redisconnection.h"
#include "connector.inl"
#include "redistrans.h"

extern "C"
{
#include "hiredis.h"    
}

using namespace std;

namespace tnet
{
    RedisClient::RedisClient(IOLoop* loop, const Address& addr, const string& password, size_t maxClients)
        : m_loop(loop)
        , m_address(addr)
        , m_password(password)
        , m_maxClients(maxClients)
    {
        m_conns.reserve(maxClients);        
    }  

    RedisClient::~RedisClient()
    {
        for(size_t i = 0; i < m_conns.size(); ++i)
        {
            RedisConnectionPtr_t conn = m_conns[i].lock();
            if(conn)
            {
                conn->shutDown();    
            }    
        }    
    }

    void RedisClient::pushConn(const RedisConnectionPtr_t& conn)
    {
        if(!conn)
        {
            return;    
        }

        conn->clearCallback();

        if(m_conns.size() > m_maxClients)
        {
            conn->shutDown();
            return;
        }

        m_conns.push_back(conn);
    }

    RedisConnectionPtr_t RedisClient::popConn()
    {
        if(m_conns.empty())
        {
            return RedisConnectionPtr_t();    
        }        

        while(!m_conns.empty())
        {
            RedisConnectionPtr_t conn = m_conns.back().lock();
            m_conns.pop_back();
            if(conn)
            {
                return conn;
            }
        }
        return RedisConnectionPtr_t();
    }

    void RedisClient::onReply(const RedisConnectionPtr_t& conn, const RedisReply& reply, const ReplyCallback_t& callback)
    {
        RedisConnectionPtr_t c = conn->shared_from_this(); 
        auto cb = std::move(callback);

        pushConn(conn);    
        
        cb(reply); 
    }

    void RedisClient::newTrans(const NewTransCallback_t& callback)
    {
        RedisConnectionPtr_t conn = popConn();
        if(conn)
        {
            RedisTransPtr_t trans = std::make_shared<RedisTrans>(shared_from_this(), conn);
            
            conn->setCallback(std::bind(&RedisTrans::onReply, trans, _1, _2));

            callback(trans, 0);
        }
        else
        { 
            conn = std::make_shared<RedisConnection>();
            
            RedisTransPtr_t trans = std::make_shared<RedisTrans>(shared_from_this(), conn);
         
            conn->connect(m_loop, m_address, m_password, std::bind(&RedisTrans::onConnect, trans, _1, _2, callback));
        }    
    }

    void RedisClient::exec(initializer_list<string> cmd, const ReplyCallback_t& callback)
    { 
        RedisConnectionPtr_t conn = popConn();
        if(conn)
        {
            conn->setCallback(std::bind(&RedisClient::onReply, 
                        shared_from_this(), _1, _2, callback));

            conn->exec(cmd);
        }   
        else
        {
            conn = std::make_shared<RedisConnection>();
            conn->setCallback(std::bind(&RedisClient::onReply, 
                        shared_from_this(), _1, _2, callback));

            conn->connect(m_loop, m_address, m_password, std::bind(&RedisClient::onConnect, shared_from_this(), _1, _2, vector<string>(cmd)));
        } 
    }

    void RedisClient::onConnect(const RedisConnectionPtr_t& conn, int status, const vector<string>& cmd)
    {
        if(status != 0)
        {
            LOG_ERROR("redis client connect fail %d", status);
            return;    
        }

        conn->exec(cmd);
    }
}
