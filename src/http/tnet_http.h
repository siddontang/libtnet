#pragma once

#include <string>

#include "tnet.h"

extern "C"
{
struct http_parser;    
}

namespace tnet
{
    class HttpConnection;
    class WsConnection;
    class HttpRequest;
    class HttpResponse;

    enum WsEvent
    { 
        Ws_OpenEvent,
        Ws_CloseEvent, 
        Ws_MessageEvent,
        Ws_PongEvent,    
        Ws_ErrorEvent,
    };

    typedef std::shared_ptr<HttpConnection> HttpConnectionPtr_t;
    typedef std::weak_ptr<HttpConnection> WeakHttpConnectionPtr_t;

    typedef std::shared_ptr<WsConnection> WsConnectionPtr_t;
    typedef std::weak_ptr<HttpConnection> WeakWsConnectionPtr_t;

    typedef std::function<int (const HttpRequest&)> AuthCallback_t;
    typedef std::function<void (const HttpConnectionPtr_t&, const HttpRequest&)> HttpCallback_t;
    typedef std::function<void (const WsConnectionPtr_t&, WsEvent, const std::string&)> WsCallback_t;

    typedef std::function<void (const HttpResponse&)> ResponseCallback_t;

    enum ParserEvent
    {
        Parser_MessageBegin,
        Parser_Url,
        Parser_StatusComplete,
        Parser_HeaderField,
        Parser_HeaderValue,
        Parser_HeadersComplete,
        Parser_Body,
        Parser_MessageComplete,
    };

    typedef std::function<int (struct http_parser*, ParserEvent event, const char*, size_t)> ParserCallback_t;
}
