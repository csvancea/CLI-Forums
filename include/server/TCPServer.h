#pragma once

#include <queue>

#include <server/TCPClient.h>
#include <common/ISelectable.h>
#include <common/Selector.h>
#include <common/Net.h>
#include <common/Errors.h>

class TCPServer : public ISelectable
{
public:
    TCPServer(const Peer& server_data, Selector& selector);
    ~TCPServer();

    TCPServer(const TCPServer&) = delete;
    TCPServer(const TCPServer&&) = delete;
    TCPServer& operator= (const TCPServer&) = delete;

    virtual int GetFileDescriptor() const override;
    virtual void Select() override;

    ECode Init();
    ECode GetPacket(Packet& packet);

    ECode Process();
    void CloseSocket();

    TCPClient *GetClient(const std::string& client_id);
    TCPClient *GetClient(int sockfd);

    ECode Kick(TCPClient *client);

private:
    Peer _serverData;
    Selector& _selector;
    std::queue<Packet> _packets;

    std::vector<TCPClient *>_clients;
};
