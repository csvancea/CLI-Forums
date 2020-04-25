#pragma once

enum class ECode
{
    OK,
    UDP_SOCKET,
    UDP_BIND,
    UDP_RECV,
    TCP_SOCKET,
    TCP_CONNECT,
    TCP_RECV,
    TCP_LISTEN,

    FD_SELECT,

    DUPLICATE,
    NONEXISTENT,

    INVALID_PACKET
};
