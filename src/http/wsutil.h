#pragma once

#include "tnet_http.h"

namespace tnet
{
    class WsUtil
    {
    public:
        static int buildRequest(HttpRequest& request);

        static HttpError handshake(const HttpRequest& request, HttpResponse& resp);
    };
    
}
