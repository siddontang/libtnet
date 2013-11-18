#pragma once

#include <stdint.h>
#include <multimap>

#include "tnet_http.h"

namespace tnet 
{ 
    class HttpClient
    {
    public:
        HttpClient(IOLoop* loop, int maxClients = 10);
        ~HttpClient();
        
        void request(const HttpRequest& request, const ResponseCallback_t& callback);    
 
    private:
        void onConnConnectEvent(const ConnectionPtr_t& conn, ConnEvent event, void* context, 
                                const HttpRequest& request, const ResponseCallback_t& callback);
        void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, void* context, const ResponseCallback_t& callback);
         
    private:
        IOLoop* m_loop;

        int m_maxClients;
        typedef std::multimap<uint32_t, WeakConnectionPtr_t> IpConn_t;
        IpConn_t m_conns;
    };
        
}
