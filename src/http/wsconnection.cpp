#include "wsconnection.h"

#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <endian.h>

#include "connection.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "stringutil.h"
#include "sockutil.h"
#include "log.h"

using namespace std;
using namespace std::placeholders;

namespace tnet
{        
    size_t WsConnection::ms_maxPayloadLen = 10 * 1024 * 1024;

    WsConnection::WsConnection(const ConnectionPtr_t& conn, const WsCallback_t& func)
    {
        m_fd = conn->getSockFd();

        m_conn = conn;
        m_func = func;
    
        m_payloadLen = 0;
        m_final = 0;
        m_opcode = 0;
        m_mask = 0;
        m_lastOpcode = 0;
    }

    WsConnection::~WsConnection()
    {
    }
    
    void WsConnection::handleError(const ConnectionPtr_t& conn, int statusCode, const string& message)
    {
        HttpResponse resp;
        resp.statusCode = statusCode;
        resp.body = message;

        conn->send(resp.dump());
    
        conn->shutDown();
    }

    static string upgradeKey = "Upgrade";
    static string upgradeValue = "websocket";
    static string connectionKey = "Connection";
    static string connectionValue = "upgrade";
    static string wsVersionKey = "Sec-WebSocket-Version";
    static string wsProtocolKey = "Sec-WebSocket-Protocol";
    static string wsAcceptKey = "Sec-WebSocket-Accept";

    int WsConnection::checkHeader(const ConnectionPtr_t& conn, const HttpRequest& request)
    {
        if(request.method != HTTP_GET)
        {
            handleError(conn, 405);

            return -1;    
        }
        
        map<string, string>::const_iterator iter = request.headers.find(upgradeKey);
        if(iter == request.headers.end() || StringUtil::lower(iter->second) != upgradeValue)
        {
            handleError(conn, 400, "Can \"Upgrade\" only to \"websocket\"");
            return -1;    
        }

        iter = request.headers.find(connectionKey);
        if(iter == request.headers.end() || StringUtil::lower(iter->second).find(connectionValue) == string::npos)
        {
            handleError(conn, 400, "\"Connection\" must be \"upgrade\"");
            return -1;
        }

        iter = request.headers.find(wsVersionKey);
        bool validVersion = false;
        if(iter != request.headers.end())
        {
            int version = atoi(iter->second.c_str());
            if(version == 13 || version == 7 || version == 8)
            {
                validVersion = true;    
            }    
        }

        if(!validVersion)
        {
            handleError(conn, 426, "Sec-WebSocket-Version: 13");
            return -1;    
        }

        return 0;
    }

    static string wsKey = "Sec-WebSocket-Key";
    static string wsMagicKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    
    static string calcAcceptKey(const HttpRequest& request)
    {
        map<string, string>::const_iterator iter = request.headers.find(wsKey);
        if(iter == request.headers.end())
        {
            return "";
        }  

        string key = iter->second + wsMagicKey;

        return StringUtil::base64Encode(StringUtil::sha1Bin(key));
    }

    int WsConnection::onHandshake(const ConnectionPtr_t& conn, const HttpRequest& request)
    {
        if(checkHeader(conn, request) != 0)
        {
            return -1;    
        }    

        HttpResponse resp;
        resp.statusCode = 101;
        resp.headers[upgradeKey] = upgradeValue;
        resp.headers[connectionKey] = upgradeKey;
        resp.headers[wsAcceptKey] = calcAcceptKey(request);

        map<string, string>::const_iterator iter = request.headers.find(wsProtocolKey);
        
        if(iter != request.headers.end())
        {
            static string sep = ",";
            vector<string> subs = StringUtil::split(iter->second, sep);

            //now we only choose first subprotocol
            if(!subs.empty())
            {
                resp.headers[wsProtocolKey] = subs[0];
            }
        }

        conn->send(resp.dump());

        m_status = FrameStart;

        m_func(shared_from_this(), Ws_OpenEvent, string());

        return 0;
    }

#define HANDLE(func) \
    ret = func(data + readLen, count - readLen); \
    readLen += (ret > 0 ? ret : 0);


    ssize_t WsConnection::onRead(const ConnectionPtr_t& conn, const char* data, size_t count)
    {
        size_t readLen = 0;
        ssize_t ret = 1;

        while(readLen < count && ret > 0)
        {
            switch(m_status)
            {
                case FrameStart:
                    HANDLE(onFrameStart);
                case FramePayloadLen:
                    HANDLE(onFramePayloadLen);
                    break;   
                case FramePayloadLen16:
                    HANDLE(onFramePayloadLen16);
                    break;
                case FramePayloadLen64:
                    HANDLE(onFramePayloadLen64);
                    break;
                case FrameMaskingKey:
                    HANDLE(onFrameMaskingKey);
                    break;
                case FrameData:
                    HANDLE(onFrameData);
                    break;
                case FrameFinal:
                    ret = handleFrameData(conn);
                    readLen += (ret > 0 ? ret : 0);
                    break;
                default:
                    return -1;
                    break; 
            }
        }
        
        if(ret > 0 && m_status == FrameFinal)
        {
            ret = handleFrameData(conn); 
        }
        
        if(ret < 0)
        {
            LOG_ERROR("onReadError");

            m_status = FrameError;
            m_func(shared_from_this(), Ws_ErrorEvent, string());
            
            //an error occur, only to shutdown connection
            conn->shutDown();    
        }
        
        return ret;
    }

    ssize_t WsConnection::onFrameStart(const char* data, size_t count)
    {
        m_frame.clear();

        char header = data[0];
        
        if(header & 0x70)
        {
            //reserved bits now not supported, abort
            return -1;    
        }

        m_final = header & 0x80;
        m_opcode = header & 0x0f;

        m_status = FramePayloadLen;
        return 1;    
    }

    ssize_t WsConnection::handleFramePayloadLen(size_t payloadLen)
    {
        m_payloadLen = payloadLen;
       
        m_frame.reserve(payloadLen);
        m_frame.clear();

        m_status = isMaskFrame() ? FrameMaskingKey : FrameData;    
        
        return 0;
    }

    ssize_t WsConnection::onFramePayloadLen(const char* data, size_t count)
    {
        uint8_t payloadLen = (uint8_t)data[0];
        
        m_mask = payloadLen & 0x80;
       
        payloadLen &= 0x7f;
        if(isControlFrame() && payloadLen >= 126)
        {
            //control frames must have payload < 126
            return -1;    
        }
        
        if(payloadLen < 126)
        {
            handleFramePayloadLen(payloadLen);
        }
        else if(payloadLen == 126)
        {
            m_status = FramePayloadLen16;    
        }
        else if(payloadLen == 127)
        {
            m_status = FramePayloadLen64;    
        }
        else
        {
            //payload error
            return -1;    
        }
    
        return 1;    
    }

    ssize_t WsConnection::tryRead(const char* data, size_t count, size_t tryReadData)
    {
        assert(m_frame.size() < tryReadData);
        size_t pendingSize = m_frame.size();
        if(pendingSize + count < tryReadData)
        {
            m_frame.append(data, count);
            return 0;    
        }

        m_frame.append(data, tryReadData - m_frame.size());

        return tryReadData - pendingSize;
    }

    ssize_t WsConnection::onFramePayloadLen16(const char* data, size_t count)
    {
        ssize_t readLen = tryRead(data, count, 2);
        if(readLen == 0)
        {
            return readLen;    
        }
       
        uint16_t payloadLen = *(uint16_t*)m_frame.data();
        //memcpy(&payloadLen, m_frame.data(), sizeof(uint16_t));
        
        if(payloadLen > ms_maxPayloadLen)
        {
            return -1;    
        }

        payloadLen = ntohs(payloadLen); 

        handleFramePayloadLen(payloadLen);

        return readLen; 
    }

    ssize_t WsConnection::onFramePayloadLen64(const char* data, size_t count)
    {
        ssize_t readLen = tryRead(data, count, 8);
        if(readLen == 0)
        {
            return readLen;    
        }

        uint64_t payloadLen = *(uint64_t*)m_frame.data();
        //memcpy(&payloadLen, m_frame.data(), sizeof(uint64_t));
        
        if(payloadLen > ms_maxPayloadLen)
        {
            return -1;
        }

        //todo ntohl64
        payloadLen = be64toh(payloadLen);
        
        handleFramePayloadLen(payloadLen);

        return readLen;
    }

    ssize_t WsConnection::onFrameMaskingKey(const char* data, size_t count)
    {
        ssize_t readLen = tryRead(data, count, 4);
        if(readLen == 0)
        {
            return 0;
        }    

        memcpy(m_maskingKey, m_frame.data(), sizeof(m_maskingKey));
        
        m_frame.clear();
        
        m_status = FrameData;

        return readLen; 
    }

    ssize_t WsConnection::onFrameData(const char* data, size_t count)
    {
        ssize_t readLen = tryRead(data, count, m_payloadLen);

        if(readLen == 0)
        {
            return 0;    
        }    

        if(isMaskFrame())
        {
            for(size_t i = 0; i < m_frame.size(); ++i)
            {
                m_frame[i] = m_frame[i] ^ m_maskingKey[i % 4];   
            }    
        }

        m_status = FrameFinal;
        return readLen;
    }

    ssize_t WsConnection::handleFrameData(const ConnectionPtr_t& conn)
    {
        uint8_t opcode;
        string data;

        if(isControlFrame())
        {
            if(!isFinalFrame())
            {
                //control frames must not be fragmented
                return -1;    
            }    
        }    
        else if(m_opcode == 0)
        {
            //continuation frame
            if(m_cache.empty())
            {
                //nothing to continue
                return -1;    
            }    

            m_cache += m_frame;

            if(isFinalFrame())
            {
                opcode = m_lastOpcode;
                data = m_cache;
                string().swap(m_cache);
            }
        }
        else
        {
            //start of new data message
            if(!m_cache.empty())
            {
                //can't start new message until the old one is finished
                return -1;    
            }    
            
            if(isFinalFrame())
            {
                opcode = m_opcode;
                data = m_frame;
            }
            else
            {
                m_lastOpcode = m_opcode;
                m_cache = m_frame;    
            }
        }

        string().swap(m_frame);

        if(isFinalFrame())
        {
            if(handleMessage(conn, opcode, data) < 0)
            {
                return -1;    
            }            
        }

        m_status = FrameStart;

        return 0;
    }

    ssize_t WsConnection::handleMessage(const ConnectionPtr_t& conn, uint8_t opcode, const string& data)
    {
        switch(opcode)
        {
            case 0x1:
                //utf-8 data
                m_func(shared_from_this(), Ws_MessageEvent, data);
                break;    
            case 0x2:
                //binary data
                m_func(shared_from_this(), Ws_MessageEvent, data);
                break;
            case 0x8:
                //clsoe
                m_func(shared_from_this(), Ws_CloseEvent, string());
                
                conn->shutDown();
                break;
            case 0x9:
                //ping
                sendFrame(true, 0xA, data);
                break;
            case 0xA:
                //pong
                m_func(shared_from_this(), Ws_PongEvent, data);
                break;
            default:
                //error
                return -1;
        }    
    } 

    void WsConnection::ping(const string& message)
    {
        sendFrame(true, 0x9, message);
    }

    void WsConnection::send(const string& message)
    {
        send(message, !isTextFrame());     
    }

    void WsConnection::send(const string& message, bool binary)
    {
        char opcode = binary ? 0x2 : 0x1;

        //for utf-8, we assume message is already utf-8
        sendFrame(true, opcode, message);
    }

    void WsConnection::close()
    {
        sendFrame(true, (char)0x8);    
    }

    void WsConnection::sendFrame(bool finalFrame, char opcode, const string& message)
    {
        ConnectionPtr_t conn = m_conn.lock();
        if(!conn)
        {
            return;    
        }

        string buf;

        opcode |= (finalFrame ? 0x80 : 0x00);
        
        buf.append((char*)&opcode, sizeof(opcode));

        char mask = isMaskFrame() ? 0x80 : 0x00;

        size_t payloadLen = message.size();

        if(payloadLen < 126)
        {
            char payload = mask | (char)payloadLen;
            buf.append((char*)&payload, sizeof(payload));
        }
        else if(payloadLen <= 0xFFFF)
        {
            char payload = mask | 126;
            buf.append((char*)&payload, sizeof(payload));
            uint16_t len = htons((uint16_t)payloadLen);

            buf.append((char*)&len, sizeof(uint16_t));
        }
        else
        {    
            char payload = mask | 127;
            buf.append((char*)&payload, sizeof(payload));
        
            uint64_t len = htobe64(payloadLen);
            buf.append((char*)&len, sizeof(uint64_t));
        }

        if(isMaskFrame())
        {
            int randomKey = rand();
            char maskingKey[4];
            memcpy(maskingKey, &randomKey, sizeof(maskingKey));

            buf.append(maskingKey, 4);

            size_t pos = buf.size();
            buf.append(message);
            for(size_t i = 0; i < buf.size() - pos; ++i)
            {
                buf[i + pos] = buf[i + pos] ^ maskingKey[i % 4];     
            }
        }
        else
        {
            buf.append(message);    
        }

        conn->send(buf);
    }

    void WsConnection::shutDown()
    {
        ConnectionPtr_t conn = m_conn.lock();
        if(conn)
        {
            conn->shutDown();    
        }    
    }
}
