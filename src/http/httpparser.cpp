#include "httpparser.h"

#include "httpreqeust.h"
#include "httpresponse.h"

namespace tnet
{
    class InitParserObj
    {
    public:
        InitParserObj()
        {
            HttpParser::initSettings();    
        }    
    };

    static InitParserObj initObj;
    
    HttpParser::HttpParser(const PaserCallback_t& callback, enum http_parser_type type)
        : m_callback(callback)
    {
        http_parser_init(&m_parser, type);

        m_parser.data = this;
   
        m_lastWasValue = true;
    }
   
    HttpParser::~HttpParser()
    {
        
    }

    void HttpParser::initSettings()
    {
        ms_settings.on_message_begin = &HttpParser::onMessageBegin;
        ms_settings.on_url = &HttpParser::onUrl;
        ms_settings.on_status_complete = &HttpParser::onStatusComplete;
        ms_settings.on_header_field = &HttpParser::onHeaderField;
        ms_settings.on_header_value = &HttpParser::onHeaderValue;
        ms_settings.on_headers_complete = &HttpParser::onHeadersComplete;
        ms_settings.on_body = &HttpParser::onBody;
        ms_settings.on_message_complete = &HttpParser::onMessageComplete;    
    }    

    int HttpParser::onMessageBegin(struct http_parser* parser)
    {
        HttpParser* p = (HttpParser*)parser->data;
        
        return p->handleMessageBegin();    
    }

    int HttpParser::onUrl(struct http_parser* parser, const char* at, size_t length)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleUrl(at, length);
    }

    int HttpParser::onStatusComplete(struct http_parser* parser)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleStatusComplete();
    }

    int HttpParser::onHeaderField(struct http_parser* parser, const char* at, size_t length)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleHeaderField(at, length);
    }

    int HttpParser::onHeaderValue(struct http_parser* parser, const char* at, size_t length)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleHeaderValue(at, length);
    }

    int HttpParser::onHeadersComplete(struct http_parser* parser)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleHeadersComplete();
    }

    int HttpParser::onBody(struct http_parser* parser, const char* at, size_t length)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleBody(at, length);
    }

    int HttpParser::onMessageComplete(struct http_parser* parser)
    {
        HttpParser* p = (HttpParser*)parser->data;
        return p->handleMessageComplete();
    }


    int HttpParser::handleMessageBegin()
    {
        m_curField.clear();
        m_curValue.clear();
        m_lastWasValue = true;
        return m_callback(&m_parser, Parser_Begin, 0);    
    }
        
    int HttpParser::handleUrl(const char* at, size_t length)
    {
        StackBuffer buf(at, length);
        return m_callback(&m_parser, Parser_Url, &buf);
    }
     
    int HttpParser::handleStatusComplete()
    {
        return 0;
    }
        
    int HttpParser::handleHeaderField(const char* at, size_t length)
    {
        if(m_lastWasValue)
        {
            if(!m_curField.empty())
            {
                pair<string, string> header = make_pair(m_curField, m_curValue);
                m_callback(&m_parser, Parser_Header, &header);        
            }
            m_curField.clear();    
            m_curValue.clear();
        }

        m_curField.append(at, length);

        m_lastWasValue = 0;

        return 0;
    }
        
    int HttpParser::handleHeaderValue(const char* at, size_t length)
    {
        m_curValue.append(at, length);
        m_lastWasValue = 1;

        return 0;
    }
        
    int HttpParser::handleHeadersComplete()
    {
        return m_callback(&m_parser, Parser_HeadersComplte, 0);
    }
        
    int HttpParser::handleBody(const char* at, size_t length)
    {
        StackBuffer buffer(at, length);
        return m_callback(&m_parser, Parser_Body, &buffer); 
    }
        
    int HttpParser::handleMessageComplete()
    {
        return m_callback(&m_parser, Parser_Complete, 0);
    }

    HttpRequestParser::HttpRequestParser()
        : m_parser(std::bind(&HttpRequestParser::onParser, this, _1, _2, _3))
    {
            
    }

    HttpRequestParser::~HttpRequestParser()
    {
        
    }

    int HttpRequestParser::onParser(struct http_parser* parser, ParserEvent event, void* context)
    {
        switch(event)
        {
            case Parser_Begin:
                m_request.clear();
                break;    
            case Parser_Url:
                {
                    StackBuffer* buf = (StackBuffer*)context;
                    m_request.url.append(buf->buffer, buf->count);
                }
                break;
            case Parser_Header:
                {
                    pair<string, string> header = *(pair<string, string>*)context;
                    m_request.setHeader(header.first, header.second);    
                } 
                break;
            case Parser_HeadersComplete:
                break;
            case Parser_Body:
                {
                    StackBuffer* buf = (StackBuffer*)context;
                    m_request.body.append(buf->buffer, buf->count);    
                }
                break;
            case Parser_Complete:
                break;
        }    
    }
}
