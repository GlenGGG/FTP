#ifndef _MANAGE_CLIENT_H
#define _MANAGE_CLIENT_H

#include "socket.h"

#define ERR_FORK "Fork failed"
#define COMMAND_LEN_MAX 1024

int manage_new_client(t_client_info *client_info, char *cwd);
int manage_registered_client(t_client_info* client_info);
int manage_client_command(t_client_info *client_info, char* command);

#endif
