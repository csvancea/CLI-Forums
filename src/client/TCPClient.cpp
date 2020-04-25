#include <client/TCPClient.h>
#include <common/Logger.h>
#include <common/Utils.h>

#define BUFFER_SIZE 2048

TCPClient::TCPClient(const Peer& server_data) : _serverData(server_data)
{

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
