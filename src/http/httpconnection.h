#pragma once

#include <map>
#include <string>

extern "C"
{
#include "http_parser.h"
}

#include "tnet_http.h"
#include "httprequest.h"

namespace tnet
{
    class HttpServer;
    class HttpRequest;
    class HttpResponse;

    //for http inner use
    class HttpConnection : public nocopyable
                     , public std::enable_shared_from_this<HttpConnection> 
    {
    public:
        friend class HttpServer;

        HttpConnection(HttpServer* server, const ConnectionPtr_t& conn);
        ~HttpConnection();

        int getSockFd() { return m_fd; }

        void send(HttpResponse& resp);
        void send(int statusCode);
        void send(int statusCode, const std::string& body);
        void send(int statusCode, const std::string& body, const std::map<std::string, std::string>& headers);

    private:
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

        bool validHeaderSize();

        void onRead(const ConnectionPtr_t& conn, const char* buffer, size_t count);

    private:    
        static struct http_parser_settings ms_settings;
         
        HttpServer* m_server;

        std::weak_ptr<Connection> m_conn;
        int m_fd;

        struct http_parser m_parser;

        HttpRequest m_request;

        //for parse http header
        std::string m_curField;
        bool m_lastWasValue;
    };    
}

