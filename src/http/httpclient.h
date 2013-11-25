#pragma once

#include <stdint.h>
#include <map>
#include <string>

extern "C"
{
#include "http_parser.h"    
}

#include "tnet_http.h"

namespace tnet 
{ 
    class HttpClient
    {
    public:
        HttpClient(IOLoop* loop, int maxClients = 10);
        ~HttpClient();
    
        //now only support ip:port    
        void request(const std::string& url, const ResponseCallback_t& callback, enum http_method method = HTTP_GET);
        void request(const std::string& url, const Headers_t& headers, const ResponseCallback_t& callback, enum http_method method = HTTP_GET); 
        void request(const std::string& url, const Headers_t& headers, const std::string& body, const ResponseCallback_t& callback, enum http_method method = HTTP_POST); 
 
    private:
        void request(HttpRequest& request, const ResponseCallback_t& callback);    
        
        void onResponse(const HttpClientConnPtr_t& conn, const HttpResponse& response, ResponseEvent event, const ResponseCallback_t& callback);
        void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context, 
                         const std::string& requestData, const ResponseCallback_t& callback);

        void pushConn(const WeakConnectionPtr_t& conn);
       
        WeakConnectionPtr_t popConn(uint32_t ip); 
         
    private:
        IOLoop* m_loop;

        int m_maxClients;
        typedef std::multimap<uint32_t, WeakConnectionPtr_t> IpConn_t;
        IpConn_t m_conns;
    };
        
}
