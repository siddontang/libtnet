#pragma once

#include <stdint.h>
#include <string>

namespace tnet
{
    class HttpUtil
    {
    public:
        static const std::string& codeReason(int code);
        static const char* methodStr(uint8_t method);
    
        static std::string escape(const std::string& src);
        static std::string unescape(const std::string& src);
    };
}

