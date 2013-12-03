#include "wsclient.h"

#include <stdio.h>
#include <string.h>

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
#include "httpconnector.h"
#include "connector.inl"

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

        HttpConnectorPtr_t conn = std::make_shared<HttpConnector>();
        conn->connect(m_loop, addr, std::bind(&WsClient::onConnect, shared_from_this(), _1, _2, request.dump(), callback));
    }

    void WsClient::onResponse(const HttpConnectorPtr_t& conn, const HttpResponse& response, ResponseEvent event, const WsCallback_t& callback)
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

    void WsClient::onConnect(const HttpConnectorPtr_t& conn, bool connected,
                             const string& requestData, const WsCallback_t& callback)
    {
        if(!connected)
        {
            LOG_ERROR("wsclient connect error");
            return;
        }

        string data = std::move(requestData);
        WsCallback_t cb = std::move(callback);            

        conn->setCallback(std::bind(&WsClient::onResponse, shared_from_this(), _1, _2, _3, cb));
        conn->send(data);
    }
}
