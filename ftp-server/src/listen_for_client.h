#ifndef _LISTEN_FOR_CLIENT
#define _LISTEN_FOR_CLIENT

# include "socket.h"

# define USAGE(arg) printf("%s port root_path\n" \
        "port:\t\tchose a port for the server, normally 21\n"\
        "root_path:\troot path for the server\n", arg)

# define EACCEPT_CLIENT "Client socket failed to connect"

int	listen_for_client(t_client_info *client_buffer, char *cwd);

#endif
