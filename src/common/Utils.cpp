#include <common/Utils.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

int Utils::DisableNeagle(int sockfd)
{
    int flag = 1;
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
}
