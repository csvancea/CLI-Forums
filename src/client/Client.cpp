#include <client/Client.h>
#include <common/Logger.h>


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
    std::string cmd;
    
    while (_keyboard.GetCommand(cmd) == ECode::OK) {
        if (cmd == "exit") {
            _running = false;
        }
    }
    return ECode::OK;
}
