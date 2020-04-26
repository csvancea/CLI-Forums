#include <common/Errors.h>

std::ostream &operator<<(std::ostream& os, ECode ec)
{
    std::string ret;

    switch (ec) {
    case ECode::OK: ret = "OK"; break;
    case ECode::UDP_SOCKET: ret = "UDP_SOCKET"; break;
    case ECode::UDP_BIND: ret = "UDP_BIND"; break;
    case ECode::UDP_RECV: ret = "UDP_RECV"; break;
    case ECode::TCP_SOCKET: ret = "TCP_SOCKET"; break;
    case ECode::TCP_BIND: ret = "TCP_BIND"; break;
    case ECode::TCP_RECV: ret = "TCP_RECV"; break;
    case ECode::TCP_SEND: ret = "TCP_SEND"; break;
    case ECode::TCP_CONNECT: ret = "TCP_CONNECT"; break;
    case ECode::TCP_LISTEN: ret = "TCP_LISTEN"; break;
    case ECode::TCP_ACCEPT: ret = "TCP_ACCEPT"; break;
    case ECode::FD_SELECT: ret = "FD_SELECT"; break;
    case ECode::SELECTOR_ADD: ret = "SELECTOR_ADD"; break;
    case ECode::SELECTOR_REMOVE: ret = "SELECTOR_REMOVE"; break;
    case ECode::NO_CLIENT: ret = "NO_CLIENT"; break;
    case ECode::DUPLICATE: ret = "DUPLICATE"; break;
    case ECode::NONEXISTENT: ret = "NONEXISTENT"; break;
    case ECode::ALREADY_SUBSCRIBED: ret = "ALREADY_SUBSCRIBED"; break;
    case ECode::INVALID_PACKET: ret = "INVALID_PACKET"; break;

    default: ret = fmt::format("unknown error ({})", static_cast<int>(ec));      
    }

    return os << ret;
}
