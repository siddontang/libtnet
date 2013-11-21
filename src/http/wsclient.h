#pragma once

#include <string>

#include "tnet_http.h"

namespace tnet
{
    class WsClient
    {
    public:
        WsClient(IOLoop* loop);
        ~WsClient();

        void connect(const std::string& url, const WsCallback_t& callback);
        void connect(const std::string& url, const Headers_t& headers, const WsCallback_t& callback);

    private:
        void connect(HttpRequest& request, const WsCallback_t& callback);

        void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context,
                         const std::string& requestData, const WsCallback_t& callback);
        void onResponse(const HttpClientConnPtr_t&, const HttpResponse&, ResponseEvent, const WsCallback_t&);

    private:
        IOLoop* m_loop;
    };    
}
