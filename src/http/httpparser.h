#pragma once

extern "C"
{
#include "http_parser.h"    
}

#include "tnet_http.h"
#include "httprequest.h"
#include "httpresonse.h"

namespace tnet
{
    enum ParserEvent
    {
        Parser_Begin,
        Parser_Url,
        Parser_Header,
        Parser_HeadersComplete,
        Parser_Body,
        Parser_Complete,
    };

    class HttpParser : public nocopyable
    {
    public:
        typedef int <void (struct http_parser*, ParserEvent, void*)> ParserCallback_t;  

        HttpParser(const ParserCallback_t& callback, enum http_parser_type type);
        ~HttpParser();
        
        static void initSettings();

        static int onMessageBegin(struct http_parser*);
        static int onUrl(struct http_parser*, const char*, size_t);
        static int onStatusComplete(struct http_parser*);
        static int onHeaderField(struct http_parser*, const char*, size_t);
        static int onHeaderValue(struct http_parser*, const char*, size_t);
        static int onHeadersComplete(struct http_parser*);
        static int onBody(struct http_parser*, const char*, size_t);
        static int onMessageComplete(struct http_parser*);
    
        int handleMessageBegin();
        int handleUrl(const char*, size_t);
        int handleStatusComplete();
        int handleHeaderField(const char*, size_t);
        int handleHeaderValue(const char*, size_t);
        int handleHeadersComplete();
        int handleBody(const char*, size_t);
        int handleMessageComplete();

    private:
        static struct http_parser_settings ms_settings;

        struct http_parser m_parser;

        std::string m_curField;
        std::string m_curValue;
        bool m_lastWasValue; 
    
        ParserCallback_t m_callback;
    };   
    
    class HttpRequestParser : public nocopyable
    {
    public:
        HttpRequestParser();
        ~HttpRequestParser();

    private:
        int onParser(struct http_parser*, ParserEvent, void*);

    private:
        HttpParser m_parser;
        
        HttpRequest m_request;    
    };
}
