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
        LOG_ERROR("Can't create socket, errcode: {}", _serverData.fd);
        return ECode::TCP_SOCKET;
    }

    Utils::DisableNeagle(_serverData.fd);

    ret = connect(_serverData.fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("TCP client can't connect to: {}:{}, errcode: {}", _serverData.ip, _serverData.port, ret);
        return ECode::TCP_CONNECT;
    }
    
    LOG_DEBUG("Connected TCP client to: {}:{}", _serverData.ip, _serverData.port);
    return ECode::OK;
}

ECode TCPClient::Announce()
{
    BitStream bs;
    bs.Write<uint8_t>(NetObj::RPC_CLIENT_ANNOUNCE);
    bs.Write(_serverData.client_id); // nu are ce cauta in _serverData

    return Send(bs);
}

ECode TCPClient::Subscribe(const std::string& topic, uint8_t sf)
{
    BitStream bs;
    bs.Write<uint8_t>(NetObj::RPC_SUBSCRIBE);
    bs.Write(topic);
    bs.Write(sf);

    return Send(bs);
}

ECode TCPClient::Unsubscribe(const std::string& topic)
{
    BitStream bs;
    bs.Write<uint8_t>(NetObj::RPC_UNSUBSCRIBE);
    bs.Write(topic);

    return Send(bs);
}

int TCPClient::GetFileDescriptor() const
{
    return _serverData.fd;
}

void TCPClient::Select()
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    uint16_t expected_bytes;

    bytes_read = recv(_serverData.fd, buffer, BUFFER_SIZE, 0);
    if (bytes_read < 0) {
        LOG_ERROR("recv failed, errcode: {}", bytes_read);
        return;
    }

    LOG_DEBUG("Received {} bytes from server: {}:{}", bytes_read, _serverData.ip, _serverData.port);

    if (bytes_read == 0) {
        // 0 bytes = gracefully closed
        _packets.emplace();
        return;
    }
    
    _fragments.Write(buffer, bytes_read);

    while (1) {
        _fragments.ResetReadPointer();
        if (_fragments.Read(expected_bytes) != sizeof(uint16_t))
            break;
        if (expected_bytes == 0)
            break;
        if (_fragments.GetRemainingBytesToRead() < expected_bytes) 
            break;

        Packet p;
        p.bs.Write(_fragments, expected_bytes);
        p.size = expected_bytes;
        p.source = _serverData;
        _packets.push(std::move(p));

        _fragments.Remove(0, expected_bytes + sizeof(uint16_t));
    }
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

    netbs.Write<uint16_t>(bs.GetBytes());
    netbs.Write(bs.GetUnderlyingBuffer().data(), bs.GetBytes());

    ret = send(_serverData.fd, netbs.GetUnderlyingBuffer().data(), netbs.GetBytes(), 0);
    if ((size_t)ret != netbs.GetBytes()) {
        return ECode::TCP_SEND;
    }

    LOG_DEBUG("Sent {} bytes", ret);

    return ECode::OK;
}
