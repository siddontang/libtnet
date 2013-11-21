#include "httpresponse.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

#include "httputil.h"

using namespace std;

namespace tnet
{

    HttpResponse::HttpResponse()
        : statusCode(200)
    {
    }

    HttpResponse::HttpResponse(int code, const Headers_t& headers, const string& body)
        : statusCode(code)
        , body(body)
        , headers(headers)
    {
        
    }

    HttpResponse::~HttpResponse()
    {
        
    }

    string HttpResponse::dump()
    {
        string str;
        
        char buf[1024];
        int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", statusCode, HttpUtil::codeReason(statusCode).c_str());    

        str.append(buf, n);
    
        n = snprintf(buf, sizeof(buf), "%d", int(body.size()));
        static const string ContentLength = "Content-Length";
        headers[ContentLength] = string(buf, n);

        map<string, string>::const_iterator it = headers.begin();
        while(it != headers.end())
        {
            n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            str.append(buf, n);
            ++it;    
        }

        str.append("\r\n");
        str.append(body);

        return str;
    }     

    void HttpResponse::setContentType(const std::string& contentType)
    {
        static const string ContentTypeKey = "Content-Type";
        headers[ContentTypeKey] = contentType;    
    }

    void HttpResponse::setKeepAlive(bool on)
    {
        static const string ConnectionKey = "Connection";
        if(on)
        {
            static const string KeepAliveValue = "Keep-Alive";
            headers[ConnectionKey] = KeepAliveValue;    
        }    
        else
        {
            static const string CloseValue = "close";
            headers[ConnectionKey] = CloseValue;    
        }
    }
    
    void HttpResponse::enableDate()
    {
        time_t now = time(NULL);
        struct tm t; 
        gmtime_r(&now, &t);
        char buf[128];
        int n = strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &t);
        static const string DateKey = "Date";
        headers[DateKey] = string(buf, n);
    }
}
