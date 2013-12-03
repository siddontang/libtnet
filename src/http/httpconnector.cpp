#include "httpconnector.h"

#include "connection.h"
#include "log.h"

#include "connector.inl"

using namespace std;

namespace tnet
{
    size_t HttpConnector::ms_maxHeaderSize = 80 * 1024;
    size_t HttpConnector::ms_maxBodySize = 10 * 1024 * 1024;
   
    void dummyCallback(const HttpConnectorPtr_t&, const HttpResponse&, ResponseEvent)
    {
        
    }

    HttpConnector::HttpConnector()
        : HttpParser(HTTP_RESPONSE)
        , Connector<HttpConnector>()
    {
        m_callback = std::bind(&dummyCallback, _1, _2, _3);
    }

    HttpConnector::~HttpConnector()
    {
    }
  
    void HttpConnector::clearCallback()
    {
        m_callback = std::bind(&dummyCallback, _1, _2, _3);
    }

    void HttpConnector::handleRead(const char* buffer, size_t count)
    {
        HttpConnectorPtr_t conn = shared_from_this();

        execute(buffer, count);
    }

    int HttpConnector::onMessageBegin()
    {
        m_response.clear();
        return 0;    
    }

    int HttpConnector::onHeader(const string& field, const string& value)
    {
        if(m_parser.nread >= ms_maxHeaderSize)
        {
            m_errorCode = 413;
            return -1;    
        }

        m_response.headers.insert(make_pair(field, value));    
        return 0;
    }

    int HttpConnector::onHeadersComplete()
    {
        m_response.statusCode = m_parser.status_code;
        return 0;    
    }

    int HttpConnector::onBody(const char* at, size_t length)
    {
        if(m_response.body.size() + length >= ms_maxBodySize)
        {
            m_errorCode = 413;
            return  -1;
        }

        m_response.body.append(at, length);    
        return 0;
    }

    int HttpConnector::onMessageComplete()
    {
        m_callback(shared_from_this(), m_response, Response_Complete);
        return 0;    
    }

    int HttpConnector::onError(const HttpError& error)
    {
        HttpResponse resp(error.statusCode);
        resp.body = error.message;
        m_callback(shared_from_this(), resp, Response_Error);
        return 0;    
    }

}
