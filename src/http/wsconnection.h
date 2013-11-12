#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include "tnet_http.h"

namespace tnet
{
    //now we wiil only support rfc6455
    //refer to tornado websocket implementation

    class Connection;
    class HttpRequest;

    class WsConnection : public nocopyable
                       , public std::enable_shared_from_this<WsConnection> 
    {
    public:
        friend class HttpServer;

        WsConnection(const ConnectionPtr_t& conn, const WsCallback_t& func);
        ~WsConnection();

        int getSockFd() const { return m_fd; }

        void ping(const std::string& message);
        void send(const std::string& message, bool binary);
        void send(const std::string& message);
        void close();
  
        void shutDown(); 

    private:    
        int onHandshake(const ConnectionPtr_t& conn, const HttpRequest& request);

        ssize_t onRead(const ConnectionPtr_t& conn, const char* data, size_t count);


        void handleError(const ConnectionPtr_t& conn, int statusCode, const std::string& message = "");
        int checkHeader(const ConnectionPtr_t& conn, const HttpRequest& request);

        bool isFinalFrame() { return m_final; }
        bool isMaskFrame() { return m_mask; }
        bool isControlFrame() { return m_opcode & 0x08; }
        bool isTextFrame() { return (m_opcode == 0) ? (m_lastOpcode == 0x1) : (m_opcode == 0x1); }
        bool isBinaryFrame() { return (m_opcode == 0) ? (m_lastOpcode == 0x2) : (m_opcode == 0x2); }
        
        ssize_t onFrameStart(const char* data, size_t count);
        ssize_t onFramePayloadLen(const char* data, size_t count);
        ssize_t onFramePayloadLen16(const char* data, size_t count);
        ssize_t onFramePayloadLen64(const char* data, size_t count);

        ssize_t onFrameMaskingKey(const char* data, size_t count);
        ssize_t onFrameData(const char* data, size_t count);

        ssize_t handleFramePayloadLen(size_t payloadLen);
        ssize_t handleFrameData(const ConnectionPtr_t& conn);
        ssize_t handleMessage(const ConnectionPtr_t& conn, uint8_t opcode, const std::string& message);
        ssize_t tryRead(const char* data, size_t count, size_t tryReadData);
    
        void sendFrame(bool finalFrame, char opcode, const std::string& message = std::string());

    private:
        enum FrameStatus
        {
            FrameStart,
            FramePayloadLen,
            FramePayloadLen16,
            FramePayloadLen64,
            FrameMaskingKey,
            FrameData,
            FrameFinal,
            FrameError,
        }; 
        
        WeakConnectionPtr_t m_conn;

        std::string m_frame;
    
        size_t m_payloadLen;

        FrameStatus m_status;

        uint8_t m_maskingKey[4];

        uint8_t m_final;
        uint8_t m_opcode;
        uint8_t m_mask;
        uint8_t m_lastOpcode;
    
        std::string m_cache;
    
        WsCallback_t m_func;

        int m_fd;
    
        static bool ms_maskOutgoing;
    };    
}

