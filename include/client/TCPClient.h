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
    ECode Send(const BitStream& bs);

private:
    Peer _serverData;
    std::queue<Packet> _packets;
};
