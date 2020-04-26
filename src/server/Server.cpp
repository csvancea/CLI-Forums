#include <server/Server.h>
#include <common/Logger.h>
#include <common/NetworkObjects.h>


Server::Server(const Peer& server_data) :  _running(true), _serverData(server_data), _UDPServer(server_data), _TCPServer(server_data, _selector)
{

}

ECode Server::Init()
{
    ECode err;

    LOG_MESSAGE("Initializing servers");
    err = _UDPServer.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init UDPServer: {}", err);
        return err;
    }

    err = _TCPServer.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init TCPServer: {}", err);
        return err;
    }


    LOG_MESSAGE("Initializing keyboard");
    err = _keyboard.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init Keyboard: {}", err);
        return err;
    }

    LOG_MESSAGE("Initializing input selector");
    _selector.Add(&_UDPServer);
    _selector.Add(&_TCPServer);
    _selector.Add(&_keyboard);

    return ECode::OK;
}

ECode Server::Run()
{
    while (_running) {
        _selector.Process();
        _TCPServer.Process();
        ProcessTCPPackets();
        ProcessUDPPackets();
        ProcessKeyboard();
        ProcessForumsMessages();   
    }

    return ECode::OK;
}

ECode Server::ReadUDPPacket(Packet& packet, PostData& data)
{
    std::string message;
    size_t bytes_read;

    NetObj::Topic Topic;
    uint8_t Type;

    NetObj::Int Int;
    NetObj::ShortReal ShortReal;
    NetObj::Float Float;
    NetObj::Message String;

    packet.bs.ResetReadPointer();

    bytes_read = packet.bs.Read(Topic);
    if (bytes_read != TOPIC_SIZE) {
        LOG_ERROR("Invalid packet: Can't parse topic name from UDP packet: expected_size={}, actual_size={}", TOPIC_SIZE, bytes_read);
        return ECode::INVALID_PACKET;
    }

    bytes_read = packet.bs.Read(Type);
    if (bytes_read != sizeof(uint8_t)) {
        LOG_ERROR("Invalid packet: Can't parse message type from UDP packet: expected_size={}, actual_size={}", sizeof(uint8_t), bytes_read);
        return ECode::INVALID_PACKET;
    }

    switch (Type) {
    case NetObj::TYPE_INT:
        bytes_read = packet.bs.Read(Int);
        if (bytes_read != MESSAGE_INT_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message int from UDP packet: expected_size={}, actual_size={}", MESSAGE_INT_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = Int.ToString();
        break;

    case NetObj::TYPE_SHORTREAL:
        bytes_read = packet.bs.Read(ShortReal);
        if (bytes_read != MESSAGE_SHORTREAL_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message shortreal from UDP packet: expected_size={}, actual_size={}", MESSAGE_SHORTREAL_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = ShortReal.ToString();
        break;

    case NetObj::TYPE_FLOAT:
        bytes_read = packet.bs.Read(Float);
        if (bytes_read != MESSAGE_FLOAT_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message float from UDP packet: expected_size={}, actual_size={}", MESSAGE_FLOAT_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = Float.ToString();
        break;

    case NetObj::TYPE_STRING:
        bytes_read = packet.bs.Read(String);
        if (bytes_read == 0) {
            LOG_ERROR("Invalid packet: Can't parse message string from UDP packet: expected_size>0, actual_size={}", bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = String.ToString();
        break;              
    }

    data.topic = Topic.ToString();
    data.type = Type;
    data.msg = message;

    return ECode::OK;
}

ECode Server::ProcessUDPPackets()
{
    ECode ret;
    Packet packet;
    
    while (_UDPServer.GetPacket(packet) == ECode::OK) {
        PostData data;

        ret = ReadUDPPacket(packet, data);
        if (ret != ECode::OK) {
            LOG_ERROR("Invalid UDP packet received. Can't be parsed: ", ret);
            continue;
        }

        ret = _forums.AddMessage(data.topic, data.msg, data.type, packet.source);
        if (ret != ECode::OK) {
            LOG_ERROR("Couldn't save the message: {}", ret);
            continue;
        }    
        LOG_MESSAGE("{}:{} - {} - {} - {}", packet.source.ip, packet.source.port, data.topic, NetObj::TypeToString(data.type), data.msg);
    }
    return ECode::OK;
}

ECode Server::ProcessTCPPackets()
{
    ECode ret;
    Packet packet;
    
    while (_TCPServer.GetPacket(packet) == ECode::OK) {
        uint8_t rpc;


        TCPClient *client = _TCPServer.GetClient(packet.source.fd);
        if (client == nullptr) {
            LOG_ERROR("TCP packet arrived from unknown source {}:{} ({})", packet.source.ip, packet.source.port, packet.source.client_id);
            LOG_ERROR("Packet discarded");
            continue;
        }

        packet.bs.ResetReadPointer();
        if (packet.bs.Read(rpc) != sizeof(uint8_t) || !NetObj::IsValidRPC(rpc)) {
            LOG_ERROR("Invalid TCP packet received from {}:{} ({}) - unknown RPC {}", packet.source.ip, packet.source.port, packet.source.client_id, rpc);
            LOG_ERROR("Packet discarded");
            continue;
        }

        if (packet.source.client_id == "" && rpc != NetObj::RPC_CLIENT_ANNOUNCE) {
            LOG_ERROR("Received RPC from unannounced client");
            LOG_ERROR("Client kicked");

            _TCPServer.Kick(client);
            continue;
        }
        
        switch (rpc) {
            case NetObj::RPC_CLIENT_ANNOUNCE:
            {
                std::string client_id;

                if (packet.bs.Read(client_id) == 0 || client_id.length() == 0) {
                    LOG_ERROR("Empty client_id received from {}:{}", packet.source.ip, packet.source.port);
                    LOG_ERROR("Client kicked");

                    _TCPServer.Kick(client);
                    continue;
                }
                if (_TCPServer.GetClient(client_id) != nullptr) {
                    LOG_ERROR("There's already a connected client with client_id={}", client_id);
                    LOG_ERROR("Client kicked");

                    _TCPServer.Kick(client);
                    continue;
                }
                if (client->GetPeer().client_id != "") {
                    LOG_ERROR("Client {}:{} ({}) already announced itself", packet.source.ip, packet.source.port, packet.source.client_id);
                    LOG_ERROR("Packet discarded");
                    continue;
                }

                client->SetClientID(client_id);
                LOG_MESSAGE("Client {}:{} ({}) connected and announced itself", packet.source.ip, packet.source.port, client_id);
                break;
            }

            case NetObj::RPC_SUBSCRIBE:
            {
                std::string topic;
                uint8_t sf;

                packet.bs.Read(topic);
                if (packet.bs.Read(sf) != sizeof(uint8_t)) {
                    LOG_ERROR("Invalid RPC_SUBSCRIBE");
                    LOG_ERROR("Packet discarded");
                    continue;
                }
                sf = !!sf;

                ret = _forums.Subscribe(packet.source.client_id, topic, sf);
                if (ret != ECode::OK) {
                    LOG_ERROR("Couldn't subscribe client {}:{} ({}) to topic={} sf={}", packet.source.ip, packet.source.port, packet.source.client_id, topic, sf);
                    LOG_ERROR("Packet discarded");
                    continue;
                }

                LOG_MESSAGE("Client {}:{} ({}) subscribed to topic={} sf={}", packet.source.ip, packet.source.port, packet.source.client_id, topic, sf);
                break;
            }

            case NetObj::RPC_UNSUBSCRIBE:
            {
                std::string topic;

                if (packet.bs.Read(topic) == 0 || topic.length() == 0) {
                    LOG_ERROR("Invalid RPC_UNSUBSCRIBE");
                    LOG_ERROR("Packet discarded");
                    continue;
                }

                ret = _forums.Unsubscribe(packet.source.client_id, topic);
                if (ret != ECode::OK) {
                    LOG_ERROR("Couldn't unsubscribe client {}:{} ({}) from topic={}", packet.source.ip, packet.source.port, packet.source.client_id, topic);
                    LOG_ERROR("Packet discarded");
                    continue;
                }

                LOG_MESSAGE("Client {}:{} ({}) unsubscribed from topic={}", packet.source.ip, packet.source.port, packet.source.client_id, topic);
                break;
            }
        }
    }
    return ECode::OK;
}

ECode Server::ProcessKeyboard()
{
    std::string cmd;
    
    while (_keyboard.GetCommand(cmd) == ECode::OK) {
        if (cmd == "exit") {
            _running = false;
        }
    }
    return ECode::OK;
}

ECode Server::ProcessForumsMessages()
{
    for (auto client : _TCPServer.GetClients()) {
        const auto& client_id = client->GetPeer().client_id;
        if (client_id == "") {
            continue;
        }

        auto topics = _forums.GetUserMessages(client_id);
        for (const auto& topic : topics) {
            for (const auto& message : topic.second) {
                BitStream bs;
                bs.Write<uint8_t>(NetObj::RPC_MESSAGE);
                bs.Write(topic.first);
                bs.Write(message.type);
                bs.Write(message.msg);
                bs.Write(static_cast<std::string>(message.source.ip));
                bs.Write(message.source.port);

                client->Send(bs);
            }
        }
    }

    return ECode::OK;
}
