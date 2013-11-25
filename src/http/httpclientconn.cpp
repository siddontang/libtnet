#include "httpclientconn.h"

#include "connection.h"
#include "log.h"

using namespace std;

namespace tnet
{
    size_t HttpClientConn::ms_maxHeaderSize = 80 * 1024;
    size_t HttpClientConn::ms_maxBodySize = 10 * 1024 * 1024;
   
    HttpClientConn::HttpClientConn(const ConnectionPtr_t& conn, const ResponseCallback_t& callback)
        : HttpParser(HTTP_RESPONSE)
        , m_conn(conn)
        , m_callback(callback)
    {
        
    }

    HttpClientConn::~HttpClientConn()
    {
    }
    
    void HttpClientConn::onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, const void* context)
    {
        switch(event)
        {
            case Conn_ReadEvent:
                {
                    const StackBuffer* buffer = (const StackBuffer*)context;
                    execute(buffer->buffer, buffer->count);    
                }
                break;    
        }    
    }

    int HttpClientConn::onMessageBegin()
    {
        m_response.clear();
        return 0;    
    }

    int HttpClientConn::onHeader(const string& field, const string& value)
    {
        if(m_parser.nread >= ms_maxHeaderSize)
        {
            m_errorCode = 413;
            return -1;    
        }

        m_response.headers.insert(make_pair(field, value));    
        return 0;
    }

    int HttpClientConn::onHeadersComplete()
    {
        m_response.statusCode = m_parser.status_code;
        return 0;    
    }

    int HttpClientConn::onBody(const char* at, size_t length)
    {
        if(m_response.body.size() + length >= ms_maxBodySize)
        {
            m_errorCode = 413;
            return  -1;
        }

        m_response.body.append(at, length);    
        return 0;
    }

    int HttpClientConn::onMessageComplete()
    {
        m_callback(shared_from_this(), m_response, Response_Complete);
        return 0;    
    }

    int HttpClientConn::onError(const HttpError& error)
    {
        HttpResponse resp(error.statusCode);
        resp.body = error.message;
        m_callback(shared_from_this(), resp, Response_Error);
        return 0;    
    }

}
