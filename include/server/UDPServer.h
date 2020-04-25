#pragma once

#include <queue>

#include <common/ISelectable.h>
#include <common/Net.h>
#include <common/Errors.h>

class UDPServer : public ISelectable
{
public:
    UDPServer(const Peer& server_data);
    UDPServer(const UDPServer&) = delete;
    UDPServer(const UDPServer&&) = delete;
    UDPServer& operator= (const UDPServer&) = delete;

    virtual int GetFileDescriptor() const override;
    virtual void Select() override;

    ECode Init();
    ECode GetPacket(Packet& packet);

private:
    Peer _serverData;
    std::queue<Packet> _packets;
};
