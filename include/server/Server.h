#pragma once

#include <server/UDPServer.h>
#include <server/TCPServer.h>
#include <server/Forums.h>

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
    struct PostData {
        std::string topic;
        uint8_t type;
        std::string msg;
    };

    ECode ReadUDPPacket(Packet& packet, PostData& data);
    ECode ProcessUDPPackets();
    ECode ProcessTCPPackets();
    ECode ProcessKeyboard();
    ECode ProcessForumsMessages();

    bool _running;
    Peer _serverData;
    Selector _selector;
    UDPServer _UDPServer;
    TCPServer _TCPServer;
    Keyboard _keyboard;
    Forums _forums;
};
