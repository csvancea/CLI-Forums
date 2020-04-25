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
        ProcessUDPPackets();
    }

    return ECode::OK;
}

ECode Server::ReadUDPPacket(Packet& packet, UDPData& data)
{
    std::string message;
    size_t bytes_read;
    packet.bs.ResetReadPointer();

    bytes_read = packet.bs.Read(data.topic);
    if (bytes_read != TOPIC_SIZE) {
        LOG_ERROR("Invalid packet: Can't parse topic name from UDP packet: expected_size={}, actual_size={}", TOPIC_SIZE, bytes_read);
        return ECode::INVALID_PACKET;
    }

    bytes_read = packet.bs.Read(data.type);
    if (bytes_read != sizeof(uint8_t)) {
        LOG_ERROR("Invalid packet: Can't parse message type from UDP packet: expected_size={}, actual_size={}", sizeof(uint8_t), bytes_read);
        return ECode::INVALID_PACKET;
    }

    switch (data.type) {
    case NetObj::TYPE_INT:
        bytes_read = packet.bs.Read(data.msg._int);
        if (bytes_read != MESSAGE_INT_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message int from UDP packet: expected_size={}, actual_size={}", MESSAGE_INT_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = data.msg._int.ToString();
        break;

    case NetObj::TYPE_SHORTREAL:
        bytes_read = packet.bs.Read(data.msg._shortreal);
        if (bytes_read != MESSAGE_SHORTREAL_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message shortreal from UDP packet: expected_size={}, actual_size={}", MESSAGE_SHORTREAL_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = data.msg._shortreal.ToString();
        break;

    case NetObj::TYPE_FLOAT:
        bytes_read = packet.bs.Read(data.msg._float);
        if (bytes_read != MESSAGE_FLOAT_SIZE) {
            LOG_ERROR("Invalid packet: Can't parse message float from UDP packet: expected_size={}, actual_size={}", MESSAGE_FLOAT_SIZE, bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = data.msg._float.ToString();
        break;

    case NetObj::TYPE_STRING:
        bytes_read = packet.bs.Read(data.msg._str);
        if (bytes_read == 0) {
            LOG_ERROR("Invalid packet: Can't parse message string from UDP packet: expected_size>0, actual_size={}", bytes_read);
            return ECode::INVALID_PACKET;                
        }

        message = data.msg._str.ToString();
        break;              
    }

    LOG_MESSAGE("{}:{} - {} - {} - {}", packet.source.ip, packet.source.port, data.topic, NetObj::TypeToString(data.type), message);
    return ECode::OK;
}

ECode Server::ProcessUDPPackets()
{
    ECode ret;
    Packet packet;
    
    while (_UDPServer.GetPacket(packet) == ECode::OK) {
        UDPData data;

        ret = ReadUDPPacket(packet, data);
        if (ret != ECode::OK) {
            LOG_ERROR("Invalid UDP packet received. Can't be parsed");
            continue;
        }
    }
    return ECode::OK;
}
