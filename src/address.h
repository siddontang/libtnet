#pragma once

#include <string>
#include <stdint.h>
#include <netinet/in.h>

namespace tnet
{

    class Address
    {
    public:
        Address(uint16_t port);
        Address(const std::string& ip, uint16_t port);
        Address(const struct sockaddr_in& addr);

        //host byte order
        uint32_t ip() const;

        //host byte order
        uint16_t port() const;

        const struct sockaddr_in& sockAddr() const { return m_addr; }
        
        std::string ipstr() const;

    private:
        struct sockaddr_in m_addr;
    };
    
}

