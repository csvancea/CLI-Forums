#pragma once

#include <queue>

#include <common/ISelectable.h>
#include <common/Net.h>
#include <common/Errors.h>

class TCPClient : public ISelectable
{
public:
    TCPClient(const Peer& server_data);
    ~TCPClient();

    TCPClient(const TCPClient&) = delete;
    TCPClient(const TCPClient&&) = delete;
    TCPClient& operator= (const TCPClient&) = delete;

    virtual int GetFileDescriptor() const override;
    virtual void Select() override;

    ECode Init();
    ECode GetPacket(Packet& packet);
    void CloseSocket();

    const Peer& GetPeer() const;
    void SetClientID(const std::string& client_id);
    ECode Send(const BitStream& bs);

private:
    Peer _serverData;
    Peer _clientData;
    std::queue<Packet> _packets;
    BitStream _fragments;
};
