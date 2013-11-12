#include "httpconnection.h"
#include "httpserver.h"
#include "log.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "connection.h"

using namespace std;

namespace tnet
{
    struct http_parser_settings HttpConnection::ms_settings;

    HttpConnection::HttpConnection(HttpServer* server, const ConnectionPtr_t& conn)
        : m_server(server)
        , m_conn(conn)
    {
        m_fd = conn->getSockFd();
        http_parser_init(&m_parser, HTTP_REQUEST);
        m_parser.data = this;

        m_lastWasValue = true;
    }

    HttpConnection::~HttpConnection()
    {
    }

    void HttpConnection::initSettings()
    {
        ms_settings.on_message_begin = &HttpConnection::onMessageBegin;
        ms_settings.on_url = &HttpConnection::onUrl;
        ms_settings.on_status_complete = &HttpConnection::onStatusComplete;
        ms_settings.on_header_field = &HttpConnection::onHeaderField;
        ms_settings.on_header_value = &HttpConnection::onHeaderValue;
        ms_settings.on_headers_complete = &HttpConnection::onHeadersComplete;
        ms_settings.on_body = &HttpConnection::onBody;
        ms_settings.on_message_complete = &HttpConnection::onMessageComplete;    
    }    

    int HttpConnection::onMessageBegin(struct http_parser* parser)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        
        return p->handleMessageBegin();    
    }

    int HttpConnection::onUrl(struct http_parser* parser, const char* at, size_t length)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleUrl(at, length);
    }

    int HttpConnection::onStatusComplete(struct http_parser* parser)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleStatusComplete();
    }

    int HttpConnection::onHeaderField(struct http_parser* parser, const char* at, size_t length)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleHeaderField(at, length);
    }

    int HttpConnection::onHeaderValue(struct http_parser* parser, const char* at, size_t length)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleHeaderValue(at, length);
    }

    int HttpConnection::onHeadersComplete(struct http_parser* parser)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleHeadersComplete();
    }

    int HttpConnection::onBody(struct http_parser* parser, const char* at, size_t length)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleBody(at, length);
    }

    int HttpConnection::onMessageComplete(struct http_parser* parser)
    {
        HttpConnection* p = (HttpConnection*)parser->data;
        return p->handleMessageComplete();
    }

     
    int HttpConnection::handleMessageBegin()
    {
        m_request.clear();
        m_curField.clear();
        m_lastWasValue = true;
        return 0;    
    }
        
    int HttpConnection::handleUrl(const char* at, size_t length)
    {
        if(!validHeaderSize())
        {
            return -1;    
        }

        m_request.url.append(at, length);
        return 0;
    }
     
    int HttpConnection::handleStatusComplete()
    {
        return 0;
    }
        
    int HttpConnection::handleHeaderField(const char* at, size_t length)
    {
        if(!validHeaderSize())
        {
            return -1;    
        }

        if(m_lastWasValue)
        {
            m_curField.clear();    
        }

        m_curField.append(at, length);

        m_lastWasValue = 0;

        return 0;
    }
        
    int HttpConnection::handleHeaderValue(const char* at, size_t length)
    {
        if(!validHeaderSize())
        {
            return -1;    
        }

        m_request.headers[m_curField].append(at, length);
        m_lastWasValue = 1;

        return 0;
    }
        
    int HttpConnection::handleHeadersComplete()
    {
        m_request.parseUrl();

        m_request.majorVersion = m_parser.http_major;
        m_request.minorVersion = m_parser.http_minor;
        m_request.method = (http_method)m_parser.method;
    
        if(m_server->onHeader(m_request) != 0)
        {
            return -1;    
        }

        return 0;
    }
        
    int HttpConnection::handleBody(const char* at, size_t length)
    {
        if(m_request.body.size() > (uint64_t)m_server->getMaxBodySize())
        {
            return -1;    
        }


        m_request.body.append(at, length);
        return 0;
    }
        
    int HttpConnection::handleMessageComplete()
    {
        if(!m_parser.upgrade)
        {
            ConnectionPtr_t conn = m_conn.lock();
            if(conn)
            {
                m_server->onRequest(shared_from_this(), m_request);
            }
            else
            {
                return -1;    
            }
        }

        return 0;
    }

    bool HttpConnection::validHeaderSize()
    {
        return (m_parser.nread <= (uint32_t)m_server->getMaxHeaderSize());
    }

    void HttpConnection::onRead(const ConnectionPtr_t& conn, const char* buffer, size_t count)
    {
        int n = http_parser_execute(&m_parser, &ms_settings, buffer, count);
        if(m_parser.upgrade)
        {
            //onWebsocket will reset conn callback, so add reference here
            HttpConnectionPtr_t hconn = shared_from_this(); 
            m_server->onWebsocket(conn, m_request, buffer + n, count - n);
        }
        else if(n != count)
        {
            HttpResponse resp;
            resp.statusCode = 500;

            conn->send(resp.dump());

            //http parser error, shutdown
            conn->shutDown();
            return;    
        }
    }

    void HttpConnection::send(HttpResponse& resp)
    {
        ConnectionPtr_t conn = m_conn.lock();
        if(!conn)
        {
            return;    
        }    

        conn->send(resp.dump());
    }

    void HttpConnection::send(int statusCode)
    {
        HttpResponse resp;
        resp.statusCode = statusCode;
        
        send(resp);    
    }

    void HttpConnection::send(int statusCode, const string& body)
    {
        HttpResponse resp;
        resp.statusCode = statusCode;
        resp.body = body;
        
        send(resp);    
    }

    void HttpConnection::send(int statusCode, const string& body, const map<string, string>& headers)
    {
        HttpResponse resp;
        resp.statusCode = statusCode;
        resp.body = body;
        
        resp.headers = headers;

        send(resp);
    }
}
