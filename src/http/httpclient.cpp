#include "httpclient.h"

#include "ioloop.h"
#include "httprequest.h"
#include "connection.h"
#include "sockutil.h"
#include "log.h"
#include "address.h"
#include "httpconnector.h"
#include "connector.inl"

using namespace std;

namespace tnet
{

    HttpClient::HttpClient(IOLoop* loop, int maxClients)
        : m_loop(loop)
        , m_maxClients(maxClients)
    {
    }
    
    HttpClient::~HttpClient()
    {
        IpConn_t::iterator iter = m_conns.begin();
        while(iter != m_conns.end())
        {
            HttpConnectorPtr_t conn = iter->second.lock();
            if(conn)
            {
                conn->shutDown();    
            }
            ++iter;    
        } 

        m_conns.clear();
    }

    void HttpClient::request(const string& url, const ResponseCallback_t& callback, enum http_method method)
    {
        HttpRequest req;
        req.url = url;
        req.method = method;
        
        request(req, callback);    
    }

    void HttpClient::request(const string& url, const Headers_t& headers, const ResponseCallback_t& callback, enum http_method method)
    {
        HttpRequest req;
        req.url = url;
        req.headers = headers;
        req.method = method;  
        
        request(req, callback);  
    }

    void HttpClient::request(const string& url, const Headers_t& headers, const string& body, const ResponseCallback_t& callback, enum http_method method)
    {
        HttpRequest req;
        req.url = url;
        req.headers = headers;
        req.method = method;
        req.body = body;
        
        request(req, callback);    
    }

 
    void HttpClient::pushConn(const HttpConnectorPtr_t& conn)
    {
        conn->clearCallback();
         
        ConnectionPtr_t c = conn->lockConn();
        if(!c)
        {
            return;
        }

        if(m_conns.size() >= m_maxClients)
        {
            conn->shutDown();
            return;
        }

        Address addr(0);
        if(SockUtil::getRemoteAddr(c->getSockFd(), addr) != 0)
        {
            conn->shutDown();
            return;    
        }

        m_conns.insert(make_pair(addr.ip(), conn));
    }

    HttpConnectorPtr_t HttpClient::popConn(uint32_t ip)
    {
        while(true)
        {
            IpConn_t::iterator iter = m_conns.find(ip);
            if(iter == m_conns.end())
            {
                return HttpConnectorPtr_t();        
            }
            else
            {
                WeakHttpConnectorPtr_t conn = iter->second;
                m_conns.erase(iter);
                HttpConnectorPtr_t c = conn.lock(); 
                if(c && c->lockConn())
                {
                    return c;    
                }   
            }
        }
    }

    void HttpClient::onResponse(const HttpConnectorPtr_t& conn, const HttpResponse& response, ResponseEvent event, const ResponseCallback_t& callback)
    { 
        //add refer
        HttpConnectorPtr_t c = conn->shared_from_this();
        auto cb = std::move(callback);

        if(event == Response_Complete)
        {
            pushConn(conn);
        }
        
        cb(response);
    }

    void HttpClient::request(HttpRequest& request, const ResponseCallback_t& callback)
    {
        request.parseUrl();

        Address addr(request.host, request.port);
    
        //now we only support ip
        HttpConnectorPtr_t conn = popConn(addr.ip());
        if(conn)
        {
            conn->setCallback(std::bind(&HttpClient::onResponse, shared_from_this(), _1, _2, _3, callback));
            conn->send(request.dump());
        }
        else
        {
            conn = std::make_shared<HttpConnector>();

            conn->connect(m_loop, addr, std::bind(&HttpClient::onConnect, shared_from_this(), _1, _2, request.dump(), callback), m_device);
        }
    }

    void HttpClient::onConnect(const HttpConnectorPtr_t& conn, bool connected, const string& requestData, const ResponseCallback_t& callback)
    {
        if(!connected) 
        {
            LOG_ERROR("httpclient connect error");
            return;
        }   

        string data = std::move(requestData);
        ResponseCallback_t cb = std::move(callback);

        conn->setCallback(std::bind(&HttpClient::onResponse, shared_from_this(), _1, _2, _3, cb));
        conn->send(data);
    }
}
