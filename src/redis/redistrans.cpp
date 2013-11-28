#include "redistrans.h"

#include <assert.h>

#include "redisconnection.h"
#include "log.h"
#include "redisclient.h"

using namespace std;

namespace tnet
{
    static void dummyCallback(const RedisReply&)
    {
        
    }

    RedisTrans::RedisTrans(const RedisClientPtr_t& client, const RedisConnectionPtr_t& conn)
        : m_client(client)
        , m_conn(conn)
        , m_transNum(0)
    {
        m_callback = std::bind(&dummyCallback, _1);
    }

    RedisTrans::~RedisTrans()
    {
        LOG_INFO("destroyed");

        RedisClientPtr_t client = m_client.lock();
        if(client)
        {
            client->pushConn(m_conn.lock());     
        } 
    }

    void RedisTrans::onReply(const RedisConnectionPtr_t& conn, const RedisReply& reply)
    {
        --m_transNum;
        assert(m_transNum >= 0);
        
        m_callback(reply);
    }

    void RedisTrans::onConnect(const RedisConnectionPtr_t& conn, int status, const NewTransCallback_t& callback)
    {
        if(status != 0)
        {
            LOG_ERROR("redis trans connect error");
            callback(shared_from_this(), status);
            return;    
        }    

        NewTransCallback_t cb = std::move(callback);
        conn->setCallback(std::bind(&RedisTrans::onReply, shared_from_this(), _1, _2));
        cb(shared_from_this(), 0);
    }

    void RedisTrans::request(initializer_list<string> cmd)
    {
        RedisConnectionPtr_t conn = m_conn.lock();
        if(!conn)
        {
            LOG_ERROR("connection was closed");
            return;
        }

        ++m_transNum;
        
        conn->exec(cmd);
    }

    void RedisTrans::begin()
    {    
        if(m_transNum > 0)
        {
            LOG_ERROR("already in trans");
            return;    
        }

        m_inTrans = true;
        static const string MultiCmd = "MULTI"; 
        request({MultiCmd});
    }
   
    void RedisTrans::exec(initializer_list<string> cmd)
    {
        if(m_transNum == 0)
        {
            begin();
        }

        request(cmd);
    }
     
    void RedisTrans::commit(const ReplyCallback_t& callback)
    {
        if(m_transNum == 0)
        {
            LOG_ERROR("not in trans, can't commit");
            return;    
        }    

        static const string ExecCmd = "EXEC";
    
        m_callback = std::bind(&RedisTrans::onCommit, shared_from_this(), _1, callback);

        request({ExecCmd});    
    }

    void RedisTrans::cancel(const ReplyCallback_t& callback)
    {
        if(m_transNum == 0)
        {
            LOG_ERROR("not in trans, can't commit");
            return;    
        }    

        static const string CancelCmd = "DISCARD";
    
        m_callback = std::bind(&RedisTrans::onCancel, shared_from_this(), _1, callback);

        request({CancelCmd});    
    }

    void RedisTrans::onCommit(const RedisReply& reply, const ReplyCallback_t& callback)
    {
        if(m_transNum != 0)
        {
            return;    
        }

        ReplyCallback_t cb = std::move(callback);
        
        m_callback = std::bind(&dummyCallback, _1);
        
        cb(reply);    
    }

    void RedisTrans::onCancel(const RedisReply& reply, const ReplyCallback_t& callback)
    {
        if(m_transNum != 0)
        {
            return;    
        }

        ReplyCallback_t cb = std::move(callback);
        
        m_callback = std::bind(&dummyCallback, _1);
        
        cb(reply);    
    }

  
}
