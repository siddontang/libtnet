#include "httputil.h"
#include <vector>
#include <string>

extern "C"
{
#include "http_parser.h"    
}

using namespace std;

namespace tnet
{
    static string unknown = "Unknown Error";
    
    class Reasons
    {
    public:
        Reasons()
        {
            m_reasons.resize(600);  
            
            m_reasons[100] = "Continue";
            m_reasons[101] = "Switching Protocols";
            m_reasons[200] = "OK";
            m_reasons[201] = "Created";
            m_reasons[202] = "Accepted";
            m_reasons[203] = "Non-Authoritative Information";
            m_reasons[204] = "No Content";
            m_reasons[205] = "Reset Content";
            m_reasons[206] = "Partial Content";
            m_reasons[300] = "Multiple Choices";
            m_reasons[301] = "Moved Permanently";
            m_reasons[302] = "Found";
            m_reasons[303] = "See Other";
            m_reasons[304] = "Not Modified";
            m_reasons[305] = "Use Proxy";
            m_reasons[307] = "Temporary Redirect";
            m_reasons[400] = "Bad Request";
            m_reasons[401] = "Unauthorized";
            m_reasons[402] = "Payment Required";
            m_reasons[403] = "Forbidden";
            m_reasons[404] = "Not Found";
            m_reasons[405] = "Method Not Allowed";
            m_reasons[406] = "Not Acceptable";
            m_reasons[407] = "Proxy Authentication Required";
            m_reasons[408] = "Request Time-out";
            m_reasons[409] = "Conflict";
            m_reasons[410] = "Gone";
            m_reasons[411] = "Length Required";
            m_reasons[412] = "Precondition Failed";
            m_reasons[413] = "Request Entity Too Large";
            m_reasons[414] = "Request-URI Too Large";
            m_reasons[415] = "Unsupported Media Type";
            m_reasons[416] = "Requested range not satisfiable";
            m_reasons[417] = "Expectation Failed";
            m_reasons[500] = "Internal Server Error";
            m_reasons[501] = "Not Implemented";
            m_reasons[502] = "Bad Gateway";
            m_reasons[503] = "Service Unavailable";
            m_reasons[504] = "Gateway Time-out";
            m_reasons[505] = "HTTP Version not supported";            
        }

        const string& getReason(int code)
        {
            if((size_t)code >= m_reasons.size())
            {
                return unknown;
            }     

            return m_reasons[code].empty() ? unknown : m_reasons[code];
        }

    private:
        vector<string> m_reasons;
    };

    static Reasons reason;

    const string& HttpUtil::codeReason(int code)
    {
        return reason.getReason(code);    
    }

    const char* HttpUtil::methodStr(unsigned char method)
    {
        return http_method_str((http_method)method);    
    }
   
    //refer to http://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12759/URI-Encoding-and-Decoding.htm
    
    const char HEX2DEC[256] = 
    {
        /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
        /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
        
        /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        
        /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        
        /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
    };
        
    std::string HttpUtil::unescape(const std::string & sSrc)
    {
        if(sSrc.empty())
        {
            return "";    
        }

        // Note from RFC1630:  "Sequences which start with a percent sign
        // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
        // for future extension"
        
        const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
        const int SRC_LEN = sSrc.length();
        const unsigned char * const SRC_END = pSrc + SRC_LEN;
        const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

        char * const pStart = new char[SRC_LEN];
        char * pEnd = pStart;

        while (pSrc < SRC_LAST_DEC)
        {
            if (*pSrc == '%')
            {
                char dec1, dec2;
                if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                    && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
                {
                    *pEnd++ = (dec1 << 4) + dec2;
                    pSrc += 3;
                    continue;
                }
            }

            *pEnd++ = *pSrc++;
        }

        // the last 2- chars
        while (pSrc < SRC_END)
            *pEnd++ = *pSrc++;

        std::string sResult(pStart, pEnd);
        delete [] pStart;
        return sResult;
    }

    // Only alphanum is safe.
    const char SAFE[256] =
    {
        /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
        /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
        
        /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        
        /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        
        /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };

    std::string HttpUtil::escape(const std::string & sSrc)
    {
        if(sSrc.empty())
        {
            return ""; 
        }

        const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
        const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
        const int SRC_LEN = sSrc.length();
        unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
        unsigned char * pEnd = pStart;
        const unsigned char * const SRC_END = pSrc + SRC_LEN;

        for (; pSrc < SRC_END; ++pSrc)
        {
            if (SAFE[*pSrc]) 
                *pEnd++ = *pSrc;
            else
            {
                // escape this char
                *pEnd++ = '%';
                *pEnd++ = DEC2HEX[*pSrc >> 4];
                *pEnd++ = DEC2HEX[*pSrc & 0x0F];
            }
        }

        std::string sResult((char *)pStart, (char *)pEnd);
        delete [] pStart;
        return sResult;
    }
}
