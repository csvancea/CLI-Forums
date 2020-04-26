#pragma once

enum class ECode
{
    OK,

    UDP_SOCKET,
    UDP_BIND,
    UDP_RECV,

    TCP_SOCKET,
    TCP_BIND,
    TCP_RECV,
    TCP_SEND,
    TCP_CONNECT,
    TCP_LISTEN,
    TCP_ACCEPT,

    FD_SELECT,

    SELECTOR_ADD,
    SELECTOR_REMOVE,
    NO_CLIENT,

    DUPLICATE,
    NONEXISTENT,

    INVALID_PACKET
};
