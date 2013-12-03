#pragma once

#include <string>

#include "tnet_http.h"

namespace tnet
{
    class WsClient : public nocopyable
                   , public std::enable_shared_from_this<WsClient>
    {
    public:
        WsClient(IOLoop* loop);
        ~WsClient();

        //now only support ip:port    
        void connect(const std::string& url, const WsCallback_t& callback);
        void connect(const std::string& url, const Headers_t& headers, const WsCallback_t& callback);

    private:
        void connect(HttpRequest& request, const WsCallback_t& callback);

        void onResponse(const HttpConnectorPtr_t&, const HttpResponse&, ResponseEvent, const WsCallback_t&);

        void onConnect(const HttpConnectorPtr_t& conn, bool connected, const std::string& requestData, const WsCallback_t& callback);

    private:
        IOLoop* m_loop;
    };    
}
