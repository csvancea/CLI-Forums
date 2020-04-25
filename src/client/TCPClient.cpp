#include <client/TCPClient.h>
#include <common/Logger.h>
#include <common/Utils.h>
#include <common/NetworkObjects.h>

#include <unistd.h>

#define BUFFER_SIZE 2048

TCPClient::TCPClient(const Peer& server_data) : _serverData(server_data)
{

}

TCPClient::~TCPClient()
{
    CloseSocket();
}

ECode TCPClient::Init()
{
    int ret;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
	address.sin_port = htons(_serverData.port);
	address.sin_addr = _serverData.ip;

    _serverData.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_serverData.fd < 0) {
        LOG_ERROR("Can't create TCP socket.");
        return ECode::TCP_SOCKET;
    }

    Utils::DisableNeagle(_serverData.fd);

    ret = connect(_serverData.fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("Can't connect TCP client to: {}:{}", _serverData.ip, _serverData.port);
        return ECode::TCP_CONNECT;
    }
    
    LOG_MESSAGE("Connected TCP client to: {}:{}", _serverData.ip, _serverData.port);
    return ECode::OK;
}

ECode TCPClient::Announce()
{
    BitStream bs;
    bs.Write<uint8_t>(NetObj::RPC_CLIENT_ANNOUNCE);
    bs.Write(_serverData.client_id); // nu are ce cauta in _serverData

    return Send(bs);
}

int TCPClient::GetFileDescriptor() const
{
    return _serverData.fd;
}

void TCPClient::Select()
{

}

ECode TCPClient::GetPacket(Packet& packet)
{
    if (_packets.empty()) {
        return ECode::NONEXISTENT;
    }

    packet = std::move(_packets.front());
    _packets.pop();
    
    return ECode::OK;
}

void TCPClient::CloseSocket()
{
    if (_serverData.fd != -1) {
        close(_serverData.fd);
        _serverData.fd = -1;
    }
}

ECode TCPClient::Send(const BitStream& bs)
{
    BitStream netbs;
    ssize_t ret;

    netbs.Write((uint16_t)bs.GetBytes());
    netbs.Write(bs.GetUnderlyingBuffer().data(), bs.GetBytes());

    ret = send(_serverData.fd, netbs.GetUnderlyingBuffer().data(), netbs.GetBytes(), 0);
    if ((size_t)ret != netbs.GetBytes()) {
        return ECode::TCP_SEND;
    }

    LOG_MESSAGE("Sent {} bytes", ret);

    return ECode::OK;
}
