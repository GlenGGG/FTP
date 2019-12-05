#ifndef _LISTEN_FOR_CLIENT
#define _LISTEN_FOR_CLIENT

#include "socket.h"

#define USAGE(arg) printf("%s server_ip port\n" \
        "server_ip:\tthe ip address of the server\n"\
        "port:\t\tchose a port for the server, normally 21\n", arg)
#define ECONNECT "Connect to server failed"

int	connect_to_server(t_client_info *client_info, char *cwd);

#endif
