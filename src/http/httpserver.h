#pragma once

#include <functional>
#include <memory>

#include <map>

#include "tnet_http.h"

extern "C"
{
#include "http_parser.h"    
}

namespace tnet
{
    class TcpServer;
    class Address;
    class WsConnection;
    class HttpRequest;
    class HttpResponse;
    class Connection;
    class HttpConnection;

    class HttpServer : public nocopyable
    {
    public:
        friend class HttpConnection;

        HttpServer(TcpServer* server);
        ~HttpServer();

        int listen(const Address& addr);
    
        void setHttpCallback(const std::string& path, const HttpCallback_t& callback);
        void setWsCallback(const std::string& path, const WsCallback_t& callback);

        void setHttpCallback(const std::string& path, const HttpCallback_t& callback, const AuthCallback_t& auth);
        void setWsCallback(const std::string& path, const WsCallback_t& callback, const AuthCallback_t& auth);
    
    private:
        void onConnEvent(const ConnectionPtr_t&, ConnEvent, const void* context);
    
        void onRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request, RequestEvent event, const void* context);
        void onWebsocket(const HttpConnectionPtr_t& conn, const HttpRequest& request, const void* context);

        void onError(const HttpConnectionPtr_t& conn, const HttpError& error);

        bool authRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request);

    private:
        TcpServer* m_server;
    
        std::map<std::string, HttpCallback_t> m_httpCallbacks;        
        std::map<std::string, WsCallback_t> m_wsCallbacks;
    
        std::map<std::string, AuthCallback_t> m_authCallbacks;
    };
    
}

