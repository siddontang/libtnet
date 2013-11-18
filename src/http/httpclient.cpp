#include "httpclient.h"

#include "ioloop.h"
#include "httprequest.h"
#include "connection.h"
#include "sockutil.h"
#include "log.h"

namespace tnet
{

    HttpClient::HttpClient(IOLoop* loop, int maxClients)
        : m_loop(loop)
        , m_maxClients(maxClients)
    {
    }
    
    HttpClient::~HttpClient()
    {
        for(size_t i = 0; i < m_conns.size(); ++i)
        {
            ConnectionPtr_t conn = m_conns[i].lock();
            if(conn)
            {
                conn->clearEventCallback();
                conn->shutDown();    
            }
        }    

        m_conns.clear();
    }

    void HttpClient::pushConn(const WeakConnectionPtr_t& conn)
    {
        ConnectionPtr_t c = conn.lock();
        if(!c)
        {
            return;    
        }

        if(m_conns.size() >= m_maxClients)
        {
            return;
        }

        Address addr;
        if(SockUtil::getRemoteAddr(c->getSockFd(), addr) != 0)
        {
            return;    
        }

        m_conns.insert(make_pair(addr.ip(), conn));
    }

    WeakConnectionPtr_t HttpClient::popConn(uint32_t ip)
    {
        HostConn_t::iterator iter = m_conns.find(ip);
        if(iter == m_conns.end())
        {
            return WeakConnectionPtr_t();        
        }
        else
        {
            WeakConnectionPtr_t conn = iter->second;
            m_conns.erase(iter);
            return conn;    
        }
        
    }

    void HttpClient::request(const HttpRequest& request, const ResponseCallback_t& callback)
    {

        Address addr(url.host, url.port);
    
        //now we only support ip
        WeakConnectionPtr_t weakConn = getConn(addr.ip());
        ConnectionPtr_t conn = weakConn.lock();
        if(conn)
        {
            conn->setEventCallback(std::bind(&HttpClient::onConnEvent, this, _1, _2, _3, callback);
            conn->send(request.dump());
        }
        else
        {
            int fd = SockUtil::create();
            conn = make_shared<Connection>(m_loop, fd); 
            conn->setEventCallback(std::bind(&HttpClient::onConnConnect, this, _1, _2, _3, request, callback);
            conn->connect(Address(request.host, request.port));    
        }
    }

    void HttpClient::onConnConnect(const ConnectionPtr_t& conn, ConnEvent event,
                                   void* context, const HttpRequest& request, const ResponseCallback_t& callback)
    {
        switch(event)
        {
            case Conn_ConnectEvent:
                conn->setEventCallback(std::bind(&HttpClient::onConnEvent, this, _1, _2, _3, request.host, callback);
                conn->send(request.dump());
                break;
            case Conn_ErrorEvent:
                break;
            case Conn_CloseEvent:
                break;
            default:
                break;    
        }    
    }

    void HttpClient::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, 
                                 void* context, const string& host, const ResponseCallback_t& callback)
    {
        switch(event)
        {
            case Conn_ReadEvent:
                break;
            case Conn_ErrorEvent:
            case Conn_CloseEvent:
                break;
            default:
                break;    
        };
    }
}
