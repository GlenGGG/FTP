#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"

int get_socket_addr(
    t_socket_info* socket_info, char* ip_addr, unsigned int port)
{
    struct protoent* protocol;
    int option_value;
    if (!(protocol = getprotobyname(PROTOTYPE_NAME)))
        return (fprintf(stderr, ERR_INIT_SOCKET), -1);
    if ((socket_info->fd = socket(AF_INET, SOCK_STREAM, protocol->p_proto))
        <= 1)
        return (perror(ERR_INIT_SOCKET), -1);
    option_value = 1;
    if (setsockopt(socket_info->fd, SOL_SOCKET, SO_REUSEADDR, &option_value,
            sizeof(int))
        == -1)
        perror(ERR_INIT_SOCKET_OPT);
    socket_info->addr.sin_family = AF_INET;
    socket_info->addr.sin_port = htons(port);
    inet_pton(
        socket_info->addr.sin_family, ip_addr, &socket_info->addr.sin_addr);
    socket_info->addrlen = sizeof(struct sockaddr_in);
    return (0);
}

int data_socket_open(
    t_socket_info* socket_info, char* ip_addr, unsigned int port)
{
    struct protoent* protocol;
    int option_value;
    if (!(protocol = getprotobyname(PROTOTYPE_NAME)))
        return (fprintf(stderr, ERR_INIT_SOCKET), -1);
    if ((socket_info->fd = socket(AF_INET, SOCK_STREAM, protocol->p_proto))
        <= 1)
        return (perror(ERR_INIT_SOCKET), -1);
    option_value = 1;
    if (setsockopt(socket_info->fd, SOL_SOCKET, SO_REUSEADDR, &option_value,
            sizeof(int))
        == -1)
        perror(ERR_INIT_SOCKET_OPT);
    socket_info->addr.sin_family = AF_INET;
    socket_info->addr.sin_port = htons(port);
    socket_info->addr.sin_addr.s_addr = INADDR_ANY;
    socket_info->addrlen = sizeof(struct sockaddr_in);
    if (bind(socket_info->fd, (const struct sockaddr*)&socket_info->addr,
            sizeof(socket_info->addr))
            == -1
        || listen(socket_info->fd, 100) == -1) {
        perror(ERR_BIND_OR_LISTEN_SOCKET);
        return (socket_close(socket_info->fd), -1);
    }
    return (0);
}

char* socket_get_ip(t_socket_info socket_info)
{
    char* str;

    if ((str = malloc(INET_ADDRSTRLEN)) == NULL)
        return (NULL);
    inet_ntop(AF_INET, &socket_info.addr.sin_addr, str, INET_ADDRSTRLEN);
    return (str);
}

int socket_close(int socket_fd)
{
    if (close(socket_fd) == -1) {
        perror(ERR_SOCKET_CLOSE);
        return (-1);
    }
    return (0);
}
