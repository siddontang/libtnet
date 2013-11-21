#pragma once

#include "tnet_http.h"
#include "httpparser.h"
#include "httpresponse.h"

namespace tnet
{

    class HttpClientConn : public HttpParser
                         , public std::enable_shared_from_this<HttpClientConn> 
    {
    public:
        friend class HttpClient;
        friend class WsClient;

        typedef std::function<void (const HttpClientConnPtr_t&, const HttpResponse&, ResponseEvent)> ResponseCallback_t; 
        
        HttpClientConn(const ConnectionPtr_t& conn, const ResponseCallback_t&);
        ~HttpClientConn();

        ConnectionPtr_t lockConn() { return m_conn.lock(); }
        WeakConnectionPtr_t getConn() { return m_conn; }

        static void setMaxHeaderSize(size_t size) { ms_maxHeaderSize = size; }
        static void setMaxBodySize(size_t size) { ms_maxBodySize = size; }
        
    private:
        void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context);
        
        int onMessageBegin();
        int onHeader(const std::string& field, const std::string& value);
        int onHeadersComplete();
        int onBody(const char* at, size_t length);
        int onMessageComplete();
        int onError(const HttpError& error);

    private:
        WeakConnectionPtr_t m_conn;
        
        HttpResponse m_response;    

        ResponseCallback_t m_callback;

        static size_t ms_maxHeaderSize;
        static size_t ms_maxBodySize;
    };
    
}
