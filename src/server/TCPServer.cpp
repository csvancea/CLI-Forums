#include <server/TCPServer.h>
#include <common/Logger.h>
#include <common/Utils.h>

#include <unistd.h>

#define MAX_TCP_CLIENTS_QUEUE 10

TCPServer::TCPServer(const Peer& server_data, Selector& selector, Forums& forums) : _serverData(server_data), _selector(selector), _forums(forums)
{

}

TCPServer::~TCPServer()
{
    auto it = _clients.begin();
    while (it != _clients.end()) {
        it = RemoveClientImpl(it).first;
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
        LOG_ERROR("Can't create socket, errcode: {}", _serverData.fd);
        return ECode::TCP_SOCKET;
    }

    Utils::DisableNeagle(_serverData.fd);

    ret = bind(_serverData.fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("Can't bind TCP socket to: {}:{}, errcode: {}", _serverData.ip, _serverData.port, ret);
        return ECode::TCP_BIND;
    }

    ret = listen(_serverData.fd, MAX_TCP_CLIENTS_QUEUE);
    if (ret < 0) {
        LOG_ERROR("Can't listen on TCP socket, errcode: {}", ret);
        return ECode::TCP_LISTEN;
    }
    
    LOG_DEBUG("Inited TCP server on: {}:{}", _serverData.ip, _serverData.port);
    return ECode::OK;
}

int TCPServer::GetFileDescriptor() const
{
    return _serverData.fd;
}

void TCPServer::Select()
{
    TCPClient *client = new TCPClient(_serverData);
    ECode ret;

    ret = client->Init();
    if (ret != ECode::OK) {
        LOG_DEBUG("Can't accept TCP connection, errcode: {}", ret);
        delete client;
        return;
    }

    ret = AddClient(client);
    if (ret != ECode::OK) {
        LOG_DEBUG("Can't add new client to pool, errcode: {}", ret);
        delete client;
        return;
    }
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
            it = RemoveClientImpl(it).first;
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

const std::vector<TCPClient *>& TCPServer::GetClients() const
{
    return _clients;
}

TCPClient *TCPServer::GetClient(const std::string& client_id)
{
    auto it = std::find_if(_clients.begin(), _clients.end(), 
            [&client_id](const TCPClient *client) { return client->GetPeer().client_id == client_id; });
    
    if (it != _clients.end()) {
        return *it;
    }
    return nullptr;
}

TCPClient *TCPServer::GetClient(int sockfd)
{
    auto it = std::find_if(_clients.begin(), _clients.end(), 
            [&sockfd](const TCPClient *client) { return client->GetFileDescriptor() == sockfd; });
    
    if (it != _clients.end()) {
        return *it;
    }
    return nullptr;
}

ECode TCPServer::AddClient(TCPClient *client)
{
    ECode ret = _selector.Add(client);
    if (ret != ECode::OK) {
        LOG_ERROR("Can't add new client to selector, errcode: {}", ret);
        return ECode::SELECTOR_ADD;
    }

    _clients.push_back(client);
    LOG_DEBUG("TCP client added!");
    return ECode::OK;
}

ECode TCPServer::RemoveClient(TCPClient *client)
{
    return RemoveClientImpl(client);
}

ECode TCPServer::RemoveClientImpl(TCPClient *client)
{
    auto it = std::find(_clients.begin(), _clients.end(), client);
    return RemoveClientImpl(it).second;
}

std::pair<std::vector<TCPClient *>::iterator, ECode> TCPServer::RemoveClientImpl(std::vector<TCPClient *>::iterator client_it)
{
    ECode err;
    
    if (client_it == _clients.end()) {
        LOG_ERROR("Couldn't find client in _clients vector. Client NOT deleted");
        return std::make_pair(client_it, ECode::NO_CLIENT);
    }

    err = _selector.Remove(*client_it);
    if (err != ECode::OK) {
        LOG_ERROR("Can't remove client from selector. Client NOT deleted");
        return std::make_pair(std::next(client_it), ECode::SELECTOR_REMOVE);
    }

    if ((*client_it)->GetPeer().client_id != "") {
        _forums.SetConnectionStatus((*client_it)->GetPeer().client_id, false);
        LOG_MESSAGE("Client ({}) disconnected.", (*client_it)->GetPeer());
    }
    
    delete *client_it;
    client_it = _clients.erase(client_it);
    LOG_DEBUG("TCP client deleted!");
    return std::make_pair(client_it, ECode::OK);
}

ECode TCPServer::Kick(TCPClient *client)
{
    return RemoveClient(client);
}
