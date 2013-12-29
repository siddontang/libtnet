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
    class HttpClient : public nocopyable
                     , public std::enable_shared_from_this<HttpClient>
    {
    public:
        HttpClient(IOLoop* loop, int maxClients = 10);
        ~HttpClient();

        void bindDevice(const std::string& device) { m_device = device; }

        //now only support ip:port    
        void request(const std::string& url, const ResponseCallback_t& callback, enum http_method method = HTTP_GET);
        void request(const std::string& url, const Headers_t& headers, const ResponseCallback_t& callback, enum http_method method = HTTP_GET); 
        void request(const std::string& url, const Headers_t& headers, const std::string& body, const ResponseCallback_t& callback, enum http_method method = HTTP_POST); 
 
    private:
        void request(HttpRequest& request, const ResponseCallback_t& callback);    
        
        void onResponse(const HttpConnectorPtr_t& conn, const HttpResponse& response, ResponseEvent event, const ResponseCallback_t& callback);

        void onConnect(const HttpConnectorPtr_t&, bool connected, const std::string& requestData, const ResponseCallback_t& callback);

        void pushConn(const HttpConnectorPtr_t& conn);
       
        HttpConnectorPtr_t popConn(uint32_t ip); 
         
    private:
        IOLoop* m_loop;

        int m_maxClients;
        typedef std::multimap<uint32_t, WeakHttpConnectorPtr_t> IpConn_t;
        IpConn_t m_conns;
    
        std::string m_device;
    };
        
}
