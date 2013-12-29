#include "httpserver.h"

#include <string>

#include "tcpserver.h"
#include "log.h"
#include "httpconnection.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "wsconnection.h"
#include "connection.h"
#include "httpparser.h"
#include "wsutil.h"

using namespace std;

namespace tnet
{
    static string rootPath = "/";

    void httpNotFoundCallback(const HttpConnectionPtr_t& conn, const HttpRequest& request)
    {
        HttpResponse resp;
        resp.statusCode = 404;
        
        conn->send(resp);      
    } 


    HttpServer::HttpServer(TcpServer* server)
        : m_server(server)
    {
    }
   
    HttpServer::~HttpServer()
    {
        
    }
     
    int HttpServer::listen(const Address& addr)
    {
        return m_server->listen(addr, std::bind(&HttpServer::onConnEvent, this, _1, _2, _3));     
    }

    void HttpServer::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context)
    {
        switch(event)
        {
            case Conn_EstablishedEvent:
                {
                    HttpConnectionPtr_t httpConn = std::make_shared<HttpConnection>(conn, 
                        std::bind(&HttpServer::onRequest, this, _1, _2, _3, _4));
            
                    conn->setEventCallback(std::bind(&HttpConnection::onConnEvent, httpConn, _1, _2, _3));
                }
                break;
            default:
                LOG_INFO("error when enter this");
                return;
        }
    }
    
    void HttpServer::setHttpCallback(const string& path, const HttpCallback_t& callback)
    {
        m_httpCallbacks[path] = callback;    
    }

    void HttpServer::setWsCallback(const string& path, const WsCallback_t& callback)
    {
        m_wsCallbacks[path] = callback;    
    }

    void HttpServer::setHttpCallback(const string& path, const HttpCallback_t& callback, const AuthCallback_t& auth)
    {
        setHttpCallback(path, callback);

        m_authCallbacks[path] = auth;
    }

    void HttpServer::setWsCallback(const string& path, const WsCallback_t& callback, const AuthCallback_t& auth)
    {
        setWsCallback(path, callback);

        m_authCallbacks[path] = auth;
    }

    void HttpServer::onError(const HttpConnectionPtr_t& conn, const HttpError& error)
    {
        conn->send(error.statusCode, error.message); 
        conn->shutDown(1000);
    }

    bool HttpServer::authRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request)
    {
        auto it = m_authCallbacks.find(request.path);
        if(it == m_authCallbacks.end())
        {
            return true;
        }

        HttpError err = (it->second)(request);
        if(err.statusCode != 200)
        {
            onError(conn, err);
            return false;
        } 
        else
        {
            return true;
        }
    }

    void HttpServer::onRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request, RequestEvent event, const void* context)
    {
        switch(event)
        {
            case Request_Upgrade:
                onWebsocket(conn, request, context);
                break;
            case Request_Error:
                onError(conn, *(HttpError*)context);
                break;
            case Request_Complete:
                {
                    map<string, HttpCallback_t>::iterator iter = m_httpCallbacks.find(request.path);
                    if(iter == m_httpCallbacks.end())
                    {
                        httpNotFoundCallback(conn, request); 
                    }
                    else
                    {
                        if(authRequest(conn, request))
                        { 
                            (iter->second)(conn, request);    
                        }
                    }
                }
                break;
            default:
                LOG_ERROR("invalid request event %d", event);
                break;
        }
    }


    void HttpServer::onWebsocket(const HttpConnectionPtr_t& conn, const HttpRequest& request, const void* context)
    {
        map<string, WsCallback_t>::iterator iter = m_wsCallbacks.find(request.path);
        if(iter == m_wsCallbacks.end())
        {
            onError(conn, 404);
        }
        else
        {
            if(!authRequest(conn, request))
            {
                return;
            }

            const StackBuffer* buffer = (const StackBuffer*)context;

            ConnectionPtr_t c = conn->lockConn();
            if(!c)
            {
                return;    
            }

            WsConnectionPtr_t wsConn(new WsConnection(c, iter->second));

            HttpResponse resp;
            HttpError error = WsUtil::handshake(request, resp); 

            if(error.statusCode != 200)
            {
                onError(conn, error);
                return;    
            }

            c->send(resp.dump());

            wsConn->onOpen(&request);

            wsConn->onRead(c, buffer->buffer, buffer->count);
            c->setEventCallback(std::bind(&WsConnection::onConnEvent, wsConn, _1, _2, _3));
           
            return;
        }
    }
}
