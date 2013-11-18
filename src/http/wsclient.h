#pragma once

#include "tnet_http.h"

extern "C"
{
#include "http_parser.h"    
}

namespace tnet
{
    class WsClient
    {
    public:
        WsClient();
        ~WsClient();

        void connect(const Address& addr, const WsCallback_t& callback);

    private:
        struct http_parser m_parser;         
    };    
}
