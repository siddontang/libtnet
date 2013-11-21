#include "wsclient.h"

#include <stdio.h>
#include <string.h>

#include "httpclientconn.h"
#include "connection.h"
#include "wsconnection.h"
#include "address.h"
#include "stringutil.h"
#include "wsutil.h"
#include "log.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "ioloop.h"
#include "sockutil.h"
#include "address.h"

using namespace std;

namespace tnet
{
    WsClient::WsClient(IOLoop* loop)
        : m_loop(loop)
    {
        
    }

    WsClient::~WsClient()
    {
        
    }

    void WsClient::connect(const string& url, const WsCallback_t& callback)
    {
        HttpRequest req;
        req.url = url;
        connect(req, callback);
    }

    void WsClient::connect(const string& url, const Headers_t& headers, const WsCallback_t& callback)
    {
        HttpRequest req;
        req.url = url;
        req.headers = headers;
        
        connect(req, callback);    
    }

    void WsClient::connect(HttpRequest& request, const WsCallback_t& callback)
    {
        WsUtil::buildRequest(request);
        
        request.parseUrl();
        
        Address addr(request.host, request.port);
        int fd = SockUtil::create();
        
        ConnectionPtr_t conn = std::make_shared<Connection>(m_loop, fd);
        conn->setEventCallback(std::bind(&WsClient::onConnEvent, this, _1, _2, _3, request.dump(), callback));
        conn->connect(addr);     
    }

    void WsClient::onResponse(const HttpClientConnPtr_t& conn, const HttpResponse& response, ResponseEvent event, const WsCallback_t& callback)
    {
        ConnectionPtr_t c = conn->lockConn();         
        if(c)
        {
            WsConnectionPtr_t wsConn = std::make_shared<WsConnection>(c, callback);
            if(response.statusCode != 101)
            {
                wsConn->onError();
                return;    
            }
            
            wsConn->onOpen(&response);
            
            c->setEventCallback(std::bind(&WsConnection::onConnEvent, wsConn, _1, _2, _3));
        }
    }

    void WsClient::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context,
                               const string& requestData, const WsCallback_t& callback)
    {
        switch(event)
        {
            case Conn_ConnectEvent:
                {
                    string data = std::move(requestData);
                    HttpClientConnPtr_t httpConn = std::make_shared<HttpClientConn>(conn, std::bind(&WsClient::onResponse, this, _1, _2, _3, callback));
                    conn->setEventCallback(std::bind(&HttpClientConn::onConnEvent, httpConn, _1, _2, _3));
                    conn->send(data);
                }   
                break;    
        }    
    }
}
