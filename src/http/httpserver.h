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
        
        void setMaxHeaderSize(int headerSize) { m_maxHeaderSize = headerSize; }
        void setMaxBodySize(int bodySize) { m_maxBodySize = bodySize; }

        int getMaxHeaderSize() { return m_maxHeaderSize; }
        int getMaxBodySize() { return m_maxBodySize; }

        int listen(const Address& addr);

        //call server receive full headers(include websocket)
        void setHeaderCallback(const HeaderCallback_t& callback) { m_headerCallback = callback; }
    
        void setHttpCallback(const std::string& path, const HttpCallback_t& callback);
        void setWsCallback(const std::string& path, const WsCallback_t& callback);

    private:
        void onConnEvent(const ConnectionPtr_t&, ConnEvent, void* context);
    
        int onHeader(const HttpRequest& request);
        void onRequest(const HttpConnectionPtr_t& conn, const HttpRequest& request);
        void onWebsocket(const ConnectionPtr_t& conn, const HttpRequest& request, const char* buf, size_t count);

        void onHttpConnEvent(const HttpConnectionPtr_t&, const ConnectionPtr_t&, ConnEvent, void* context);
        void onWsConnEvent(const WsConnectionPtr_t&, const ConnectionPtr_t&, ConnEvent, void* context);

    private:
        TcpServer* m_server;
    
        int m_maxHeaderSize;
        int m_maxBodySize;
    
        std::map<std::string, HttpCallback_t> m_httpCallbacks;        

        std::map<std::string, WsCallback_t> m_wsCallbacks;

        HeaderCallback_t m_headerCallback;
    };
    
}

