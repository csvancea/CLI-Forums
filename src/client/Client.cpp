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

    LOG_MESSAGE("Initializing client");
    err = _TCPClient.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init TCPClient: {}", err);
        return err;
    }

    err = _TCPClient.Announce();
    if (err != ECode::OK) {
        LOG_ERROR("Can't send initial data to server: {}", err);
        return err;
    }

    LOG_MESSAGE("Initializing keyboard");
    err = _keyboard.Init();
    if (err != ECode::OK) {
        LOG_ERROR("Can't init Keyboard: {}", err);
        return err;
    }

    LOG_MESSAGE("Initializing input selector");
    _selector.Add(&_TCPClient);
    _selector.Add(&_keyboard);

    return ECode::OK;
}

ECode Client::Run()
{
    while (_running) {
        _selector.Process();
        ProcessKeyboard();
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
                        LOG_MESSAGE("Gonna subscribe to topic={} with sf={}", topic, sf);
                    }
                    else {
                        LOG_ERROR("Couldn't subscribe: {}", err);
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
                        LOG_MESSAGE("Gonna unsubscribe from topic={}", topic);
                    }
                    else {
                        LOG_ERROR("Couldn't unsubscribe: {}", err);
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
