#pragma once

#include <server/UDPServer.h>
#include <server/TCPServer.h>

#include <common/Errors.h>
#include <common/Keyboard.h>
#include <common/Selector.h>
#include <common/NetworkObjects.h>
#include <common/Net.h>

class Server
{
public:
    Server(const Peer& server_data);
    Server(const Server&) = delete;
    Server(const Server&&) = delete;
    Server& operator= (const Server&) = delete;

    ECode Init();
    ECode Run();

private:
    struct UDPData {
        NetObj::Topic topic;
        uint8_t type;
        struct {
            NetObj::Int _int;
            NetObj::ShortReal _shortreal;
            NetObj::Float _float;
            NetObj::Message _str;
        } msg;
    };

    ECode ReadUDPPacket(Packet& packet, UDPData& data);
    ECode ProcessUDPPackets();

    bool _running;
    Peer _serverData;
    UDPServer _UDPServer;
    TCPServer _TCPServer;
    Keyboard _keyboard;

    Selector _selector;
};
