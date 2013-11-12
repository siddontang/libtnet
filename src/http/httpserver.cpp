#include "httpserver.h"

#include <string>

#include "tcpserver.h"
#include "log.h"
#include "httpconnection.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "wsconnection.h"
#include "connection.h"

using namespace std;

namespace tnet
{
    const int DefaultMaxHeaderSize = 4 * 1024;
    const int DefaultMaxBodySize = 1024 * 1024;

    static string rootPath = "/";

    void httpNotFoundCallback(const HttpConnectionPtr_t& conn, const HttpRequest& request)
    {
        HttpResponse resp;
        resp.statusCode = 404;
        
        conn->send(resp);      
    } 

    int dummyHeaderCallback(const HttpRequest&)
    {
        return 0;    
    }

    HttpServer::HttpServer(TcpServer* server)
        : m_server(server)
        , m_maxHeaderSize(DefaultMaxHeaderSize)
        , m_maxBodySize(DefaultMaxBodySize)
    {
        HttpConnection::initSettings();
    
        m_httpCallbacks[rootPath] = std::bind(&httpNotFoundCallback, _1, _2);
    
        m_headerCallback = std::bind(&dummyHeaderCallback, _1);
    }
   
    HttpServer::~HttpServer()
    {
        
    }
     
    int HttpServer::listen(const Address& addr)
    {
        return m_server->listen(addr, std::bind(&HttpServer::onConnEvent, this, _1, _2, _3));     
    }

    void HttpServer::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, void* context)
    {
        switch(event)
        {
            case Conn_EstablishedEvent:
                {
                    HttpConnectionPtr_t httpConn(new HttpConnection(this, conn));
                    conn->setEventCallback(std::bind(&HttpServer::onHttpConnEvent, this, httpConn, _1, _2, _3));
                }
                break;
            default:
                LOG_INFO("error when enter this");
                return;
        }
    }

    void HttpServer::onHttpConnEvent(const HttpConnectionPtr_t& httpConn, const ConnectionPtr_t& conn, ConnEvent event, void* context)
    {
        switch(event)
        {
            case Conn_ReadEvent:
                {
                    StackBuffer* b = static_cast<StackBuffer*>(context);
                    httpConn->onRead(conn, b->buffer, b->count);
                }
                break;
            default:
                break;    
        }    
    }

    void HttpServer::onWsConnEvent(const WsConnectionPtr_t& wsConn, const ConnectionPtr_t& conn, ConnEvent event, void* context)
    {
        switch(event)
        {
            case Conn_ReadEvent:
                {
                    StackBuffer* b = static_cast<StackBuffer*>(context);
                    wsConn->onRead(conn, b->buffer, b->count);
                }
                break;
            default:
                break;    
        }
    }

    void HttpServer::setHttpCallback(const string& path, const HttpCallback_t& callback)
    {
        m_httpCallbacks[path] = callback;    
    }

    void HttpServer::onRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request)
    {
        map<string, HttpCallback_t>::iterator iter = m_httpCallbacks.find(request.path);
        if(iter == m_httpCallbacks.end())
        {
            m_httpCallbacks[rootPath](conn, request);  
        }
        else
        {
            (iter->second)(conn, request);    
        }
    }

    void HttpServer::setWsCallback(const string& path, const WsCallback_t& callback)
    {
        m_wsCallbacks[path] = callback;    
    }

    void HttpServer::onWebsocket(const ConnectionPtr_t& conn, const HttpRequest& request, const char* buffer, size_t count)
    {
        map<string, WsCallback_t>::iterator iter = m_wsCallbacks.find(request.path);
        if(iter == m_wsCallbacks.end())
        {
            conn->shutDown();
        }
        else
        {
            WsConnectionPtr_t wsConn(new WsConnection(conn, iter->second));

            if(wsConn->onHandshake(conn, request) == 0)
            { 
                wsConn->onRead(conn, buffer, count);
                conn->setEventCallback(std::bind(&HttpServer::onWsConnEvent, this, wsConn, _1, _2, _3));
            }
            else
            {
                conn->shutDown();    
            }
           
            return;
        }
    }

    int HttpServer::onHeader(const HttpRequest& request)
    {
        if(m_headerCallback(request) != 0)
        {
            return -1;         
        }    

        return 0;
    }
}
