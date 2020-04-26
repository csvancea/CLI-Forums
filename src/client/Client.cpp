#include <client/Client.h>
#include <common/Logger.h>
#include <common/NetworkObjects.h>

#include <sstream>


Client::Client(const Peer& server) :  _running(true), _serverData(server), _TCPClient(server)
{

}

ECode Client::Init()
{
    ECode err;

    LOG_DEBUG("Initializing client");
    err = _TCPClient.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init TCPClient, errcode: {}", err);
        return err;
    }

    err = _TCPClient.Announce();
    if (err != ECode::OK) {
        LOG_ERROR("Can't send initial data to server, errcode: {}", err);
        return err;
    }

    LOG_DEBUG("Initializing keyboard");
    err = _keyboard.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init Keyboard, errcode: {}", err);
        return err;
    }

    LOG_DEBUG("Initializing input selector");
    _selector.Add(&_TCPClient);
    _selector.Add(&_keyboard);

    LOG_DEBUG("Client inited!");
    return ECode::OK;
}

ECode Client::Run()
{
    while (_running) {
        _selector.Process();
        ProcessTCPPackets();
        ProcessKeyboard();
    }

    return ECode::OK;
}

ECode Client::ProcessTCPPackets()
{
    Packet packet;
    
    while (_TCPClient.GetPacket(packet) == ECode::OK) {
        uint8_t rpc;

        if (packet.size == 0) {
            LOG_DEBUG("Server shutdown.");
            _running = false;
            break;
        }

        packet.bs.ResetReadPointer();
        if (packet.bs.Read(rpc) != sizeof(uint8_t) || !NetObj::IsValidRPC(rpc)) {
            LOG_ERROR("Invalid TCP packet received from {} - unknown RPC {}", packet.source, rpc);
            LOG_ERROR("Packet discarded");
            continue;
        }
        
        switch (rpc) {
            case NetObj::RPC_MESSAGE:
            {
                std::string topic;
                uint8_t type;
                std::string msg;
                std::string ip;
                uint16_t port;

                packet.bs.Read(topic);
                packet.bs.Read(type);
                packet.bs.Read(msg);
                packet.bs.Read(ip);
                if (packet.bs.Read(port) != sizeof(uint16_t)) {
                    LOG_ERROR("Incomplete RPC_MESSAGE received from {}", packet.source);
                    LOG_ERROR("Packet discarded");
                    continue;
                }

                LOG_MESSAGE("{}:{} - {} - {} - {}", ip, port, topic, NetObj::TypeToString(type), msg);
                break;
            }
        }
    }
    return ECode::OK;
}

ECode Client::ProcessKeyboard()
{
    ECode err;
    std::string cmdline;

    while (_keyboard.GetCommand(cmdline) == ECode::OK) {
        std::stringstream ss(cmdline);
        std::string cmd;

        if (ss >> cmd) {
            if (cmd == "exit") {
                _running = false;
            }
            else if (cmd == "subscribe") {
                std::string topic;
                int sf;

                if (ss >> topic >> sf) {
                    err = _TCPClient.Subscribe(topic, sf);
                    if (err == ECode::OK) {
                        LOG_DEBUG("Sent subscribe RPC: topic={} with sf={}", topic, sf);
                        LOG_MESSAGE("topic subscribed");
                    }
                    else {
                        LOG_ERROR("Couldn't send subscribe RPC, errcode: {}", err);
                    }
                } 
                else {
                    LOG_ERROR("subscribe <topic> <sf>");
                }
            }
            else if (cmd == "unsubscribe") {
                std::string topic;

                if (ss >> topic) {
                    err = _TCPClient.Unsubscribe(topic);
                    if (err == ECode::OK) {
                        LOG_DEBUG("Sent unsubscribe RPC: topic={}", topic);
                        LOG_MESSAGE("topic unsubscribed");
                    }
                    else {
                        LOG_ERROR("Couldn't send unsubscribe RPC, errcode: {}", err);
                    }
                }
                else {
                    LOG_ERROR("unsubscribe <topic>");
                }          
            }
            else {
                LOG_ERROR("Unknown command: {}", cmdline);
            }
        }
        else {
            LOG_ERROR("Can't parse command name from input: {}", cmdline);
        }
    }
    return ECode::OK;
}
