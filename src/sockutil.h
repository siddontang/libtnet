#pragma once

#include <string>
#include <stdint.h>

namespace tnet
{
    class Address;
    class SockUtil
    {
    public:
        static int create();
        static int bindAndListen(const Address& addr);
        static int connect(int sockFd, const Address& addr);

        static int setNoDelay(int sockFd, bool on);
        
        static int setReuseable(int sockFd, bool on);
        static int setKeepAlive(int sockFd, bool on);

        static int getLocalAddr(int sockFd, Address& addr);
        static int getRemoteAddr(int sockFd, Address& addr);

        static int getSockError(int sockFd);

        static uint32_t getHostByName(const std::string& host);    

        static int bindDevice(int sockFd, const std::string& device);
    };

}

