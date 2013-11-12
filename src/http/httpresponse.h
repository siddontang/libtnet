#pragma once

#include <string>
#include <map>

namespace tnet
{
    class HttpResponse
    {
    public:
        HttpResponse();

        typedef std::map<std::string, std::string> Headers_t;
        HttpResponse(int code, const Headers_t& headers, const std::string& body = "");
        ~HttpResponse();    

        void setContentType(const std::string& contentType);
        void setKeepAlive(bool on);

        void enableDate();

        //generate http response text
        std::string dump();
   
        int statusCode;
        std::string body;
        std::map<std::string, std::string> headers;
    };
    
}

