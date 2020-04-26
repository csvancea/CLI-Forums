#pragma once

#include <client/TCPClient.h>

#include <common/Errors.h>
#include <common/Keyboard.h>
#include <common/Selector.h>
#include <common/Net.h>

class Client
{
public:
    Client(const Peer& server_data);
    Client(const Client&) = delete;
    Client(const Client&&) = delete;
    Client& operator= (const Client&) = delete;

    ECode Init();
    ECode Run();

private:
    ECode ProcessTCPPackets();
    ECode ProcessKeyboard();

    bool _running;
    Peer _serverData;
    TCPClient _TCPClient;
    Keyboard _keyboard;

    Selector _selector;
};
