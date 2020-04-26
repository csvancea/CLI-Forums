#include <server/TCPClient.h>
#include <common/Logger.h>
#include <common/Utils.h>

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
    sockaddr_in client_addr;
    socklen_t sockaddr_size = sizeof(sockaddr_in);

    _clientData.fd = accept(_serverData.fd, (struct sockaddr *) &client_addr, &sockaddr_size);
    if (_clientData.fd < 0) {
        LOG_ERROR("Can't accept new TCP connection.");
        return ECode::TCP_ACCEPT;
    }

    _clientData.ip = IP(client_addr.sin_addr);
    _clientData.port = ntohs(client_addr.sin_port);
    
    Utils::DisableNeagle(_clientData.fd);

    LOG_MESSAGE("New TCP client connected from: {}:{}", _clientData.ip, _clientData.port);
    return ECode::OK;
}

int TCPClient::GetFileDescriptor() const
{
    return _clientData.fd;
}

void TCPClient::Select()
{
    /**
     * TODO:
     * ----
     * 
     * 1. Ia un buffer temporar care sa aiba dimensiunea maxima posibila a unui pachet valid (> ~1600)
     * 2. Scrie in el fragmente de pachete pana cand se reconstruieste pachetul initial de la client
     * 3. Push la pachet in queue si asteapta urmatoarele fragmente
     */

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    uint16_t expected_bytes;

    bytes_read = recv(_clientData.fd, buffer, BUFFER_SIZE, 0);
    if (bytes_read < 0) {
        LOG_ERROR("recv failed with error code: {}", bytes_read);
        return;
    }

    LOG_MESSAGE("Received {} bytes from TCPClient: {}:{} ({})", bytes_read, _clientData.ip, _clientData.port, _clientData.client_id);

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
        p.source = _clientData;
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
    if (_clientData.fd != -1) {
        close(_clientData.fd);
        _clientData.fd = -1;
    }
}

const Peer& TCPClient::GetPeer() const
{
    return _clientData;
}

void TCPClient::SetClientID(const std::string& client_id)
{
    _clientData.client_id = client_id;
}

ECode TCPClient::Send(const BitStream& bs)
{
    BitStream netbs;
    ssize_t ret;

    netbs.Write((uint16_t)bs.GetBytes());
    netbs.Write(bs.GetUnderlyingBuffer().data(), bs.GetBytes());

    ret = send(_clientData.fd, netbs.GetUnderlyingBuffer().data(), netbs.GetBytes(), 0);
    if ((size_t)ret != netbs.GetBytes()) {
        return ECode::TCP_SEND;
    }

    LOG_MESSAGE("Sent {} bytes", ret);

    return ECode::OK;
}
