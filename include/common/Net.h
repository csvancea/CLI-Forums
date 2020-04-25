
#pragma once

#include <arpa/inet.h>

#include <common/BitStream.h>
#include <fmt/ostream.h>

class IP 
{
public:
    IP () : IP(INADDR_NONE) { }
    IP (uint32_t ip) 
    {
        addr.s_addr = ip;
        addrString.assign(inet_ntoa(addr));
    }
    IP (in_addr addr) : IP(addr.s_addr) { }
    IP (const uint8_t *ip) : IP(*reinterpret_cast<const uint32_t *>(ip)) { }
    IP (const char *ip) : IP (inet_addr(ip)) { }
    IP (const std::string& ip) : IP (ip.c_str()) { }

    operator in_addr() const { return addr; }
    explicit operator uint32_t() const { return addr.s_addr; }
    explicit operator const std::string&() const { return addrString; }
    explicit operator const char *() const { return addrString.c_str(); }

    inline bool operator==(const IP& rhs) const { return this->addr.s_addr == rhs.addr.s_addr; }
    inline bool operator!=(const IP& rhs) const { return this->addr.s_addr != rhs.addr.s_addr; }

    inline bool operator==(uint32_t rhs) const { return this->addr.s_addr == rhs; }
    inline bool operator!=(uint32_t rhs) const { return this->addr.s_addr != rhs; }

    friend std::ostream& operator<<(std::ostream& os, const IP& ip) {
        return os << ip.addrString;
    }

private:
    in_addr addr;
    std::string addrString;
};

struct Peer
{
    Peer() : client_id("unknown"), ip(INADDR_NONE), port(0), fd(-1) { }

    std::string client_id;
    IP ip;
    int port;
    int fd;

    bool IsValid() const {
        if (client_id.length() == 0 || client_id.length() > 10)
            return false;
        
        if (ip == INADDR_NONE)
            return false;

        if (port < 1 || port > 65535)
            return false;

        return true;
    }
};

struct Packet
{
    Packet() : size(0) { }

    Peer source;
    BitStream bs;
    size_t size;
};
