#include <server/UDPServer.h>
#include <common/Logger.h>

#define BUFFER_SIZE 2048

UDPServer::UDPServer(const Peer& server_data) : _serverData(server_data)
{

}

ECode UDPServer::Init()
{
    int ret;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
	address.sin_port = htons(_serverData.port);
	address.sin_addr = _serverData.ip;

    _serverData.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_serverData.fd < 0) {
        LOG_ERROR("Can't create UDP socket.");
        return ECode::UDP_SOCKET;
    }

    ret = bind(_serverData.fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("Can't bind UDP socket to: {}:{}", _serverData.ip, _serverData.port);
        return ECode::UDP_BIND;
    }
    
    LOG_MESSAGE("Inited UDP server on: {}:{}", _serverData.ip, _serverData.port);
    return ECode::OK;
}

int UDPServer::GetFileDescriptor() const
{
    return _serverData.fd;
}

void UDPServer::Select()
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    sockaddr_in client_addr;
    socklen_t sockaddr_size = sizeof(sockaddr_in);

    bytes_read = recvfrom(_serverData.fd, buffer, BUFFER_SIZE, 0, (sockaddr *) &client_addr, &sockaddr_size);
    if (bytes_read < 0) {
        LOG_ERROR("recvfrom failed with error code: {}", bytes_read);
        return;
    }

    /**
     * Validarea pachetului se face la procesare.
     * Oricum fiind UDP, e hit or miss. Fie a ajuns pachetul intreg, fie s-a pierdut o bucata pe drum si a devenit invalid
     */
    Packet p;
    p.bs.Write(buffer, bytes_read);
    p.size = bytes_read;
    p.source.ip = IP(client_addr.sin_addr);
    p.source.port = ntohs(client_addr.sin_port);

    _packets.push(std::move(p));
}

ECode UDPServer::GetPacket(Packet& packet)
{
    if (_packets.empty()) {
        return ECode::NONEXISTENT;
    }

    packet = std::move(_packets.front());
    _packets.pop();
    
    return ECode::OK;
}
