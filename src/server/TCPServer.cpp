#include <server/TCPServer.h>
#include <common/Logger.h>
#include <common/Utils.h>

#include <unistd.h>

#define MAX_TCP_CLIENTS_QUEUE 10

TCPServer::TCPServer(const Peer& server_data, Selector& selector) : _serverData(server_data), _selector(selector)
{

}

TCPServer::~TCPServer()
{
    for (auto client : _clients) {
        delete client;
    }

    CloseSocket();
}

ECode TCPServer::Init()
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

    ret = bind(_serverData.fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("Can't bind TCP socket to: {}:{}", _serverData.ip, _serverData.port);
        return ECode::TCP_BIND;
    }

    ret = listen(_serverData.fd, MAX_TCP_CLIENTS_QUEUE);
    if (ret < 0) {
        LOG_ERROR("Can't listen on TCP socket");
        return ECode::TCP_LISTEN;
    }
    
    LOG_MESSAGE("Inited TCP server on: {}:{}", _serverData.ip, _serverData.port);
    return ECode::OK;
}

int TCPServer::GetFileDescriptor() const
{
    return _serverData.fd;
}

void TCPServer::Select()
{
    TCPClient *client = new TCPClient(_serverData);
    ECode ret = client->Init();

    if (ret != ECode::OK) {
        LOG_MESSAGE("Can't accept TCP connection: {}", ret);
        delete client;
        return;
    }

    ret = _selector.Add(client);
    if (ret != ECode::OK) {
        LOG_ERROR("Can't add new client to selector: {}", ret);
        delete client;
        return;
    }

    _clients.push_back(client);
    LOG_MESSAGE("TCP client added!");
}

ECode TCPServer::GetPacket(Packet& packet)
{
    if (_packets.empty()) {
        return ECode::NONEXISTENT;
    }

    packet = std::move(_packets.front());
    _packets.pop();
    
    return ECode::OK;
}

ECode TCPServer::Process()
{
    auto it = _clients.begin();

    while (it != _clients.end()) {
        Packet packet;
        auto remove = false;
        auto client = *it; 

        while (client->GetPacket(packet) == ECode::OK) {
            if (packet.size == 0) {
                remove = true;
                break;
            }

            _packets.push(std::move(packet));
        }

        if (remove) {
            _selector.Remove(client);
            it = _clients.erase(it);
            delete client;

            LOG_MESSAGE("TCP client removed!");
        } else {
            it++;
        }
    }

    return ECode::OK;
}

void TCPServer::CloseSocket()
{
    if (_serverData.fd != -1) {
        close(_serverData.fd);
        _serverData.fd = -1;
    }
}
