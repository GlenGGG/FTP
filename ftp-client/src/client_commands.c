#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client_commands.h"
#include "socket.h"

t_command_alias client_commands[35] = { { "USER", &client_command_user },
    { "PASS", &client_command_pass }, { "ACCT", &client_command_unimplemented },
    { "CWD", &client_command_cwd }, { "CDUP", &client_command_cdup },
    { "SMNT", &client_command_unimplemented },
    { "REIN", &client_command_unimplemented }, { "QUIT", &client_command_quit },
    { "PORT", &client_command_port }, { "PASV", &client_command_pasv },
    { "TYPE", &client_command_unimplemented },
    { "STRU", &client_command_unimplemented },
    { "MODE", &client_command_unimplemented }, { "RETR", &client_command_retr },
    { "STOR", &client_command_stor }, { "STOU", &client_command_unimplemented },
    { "APPE", &client_command_unimplemented },
    { "ALLO", &client_command_unimplemented },
    { "REST", &client_command_unimplemented },
    { "RNFR", &client_command_unimplemented },
    { "RNTO", &client_command_unimplemented },
    { "ABOR", &client_command_unimplemented },
    { "DELE", &client_command_unimplemented },
    { "RMD", &client_command_unimplemented },
    { "MDK", &client_command_unimplemented }, { "PWD", &client_command_pwd },
    { "LIST", &client_command_list }, { "NLST", &client_command_unimplemented },
    { "SITE", &client_command_unimplemented },
    { "SYST", &client_command_unimplemented },
    { "STAT", &client_command_unimplemented }, { "HELP", &client_command_help },
    { "?", &client_command_help }, { "NOOP", &client_command_noop }, { 0 } };

int client_command_unimplemented(t_client_info* client_info, char* argument)
{
    printf("Yet to implement\n");
    (void)argument;
    return (0);
}

int client_remote_connection(t_client_info* client_info)
{
    client_info->data.addrlen = sizeof(struct sockaddr);
    if (client_info->mode == PASSIVE) {
        usleep(5000);
        struct protoent* pe;
        if (!(pe = getprotobyname(PROTOTYPE_NAME))
            || (client_info->data.fd = socket(client_info->data.addr.sin_family,
                    SOCK_STREAM, pe->p_proto))
                == -1)
            return (-1);
        if (connect(client_info->data.fd,
                (struct sockaddr*)&client_info->data.addr,
                client_info->data.addrlen)
            == -1)
            return (-1);
    } else if (client_info->mode == ACTIVE) {
        if ((client_info->data.fd = accept(client_info->dataListen.fd,
                 (struct sockaddr*)&client_info->data.addr,
                 &client_info->data.addrlen))
            == -1) {
            return (-1);
        }
    }
    return (0);
}

char* client_get_real_path(char* path, char* root)
{
    char* real_path;
    char ret_path[4096];

    if (path[0] == '/') {
        sprintf(ret_path, "%s/%s", root, path);
        real_path = realpath(strdup(ret_path), NULL);
    } else if (!(real_path = realpath(path, NULL))) {
        return (NULL);
    }
    return (real_path);
}

int client_command_user(t_client_info* client_info, char* argument)
{
    return (0);
}

static int client_command_list_close(t_client_info* client_info)
{
    if (client_info->mode == DEFAULT)
        return (0);
    if (socket_close(client_info->data.fd) == -1)
        return (-1);
    /* if (socket_close(client_info->data.fd))
     *   return (-1); */
    return (0);
}

int client_command_list(t_client_info* client_info, char* argument)
{
    (void)argument;
    if (client_remote_connection(client_info) == -1) {
        perror("Client to Server Conection Failed");
        return (-1);
    }
    client_info->data_in_use = true;
    int read_len = 0;
    char buf[4096];
    while ((read_len = read(client_info->data.fd, buf, 4096)) > 0) {
        static int cnt = 0;
        buf[read_len] = 0;
        if (cnt == 0)
            pthread_mutex_lock(&client_info->print_mutex);
        printf("%s", buf);
        int buf_len = strlen(buf);
        if (buf[buf_len - 1] == '\n' && buf[buf_len - 2] == '\r')
            pthread_mutex_unlock(&client_info->print_mutex);
        ++cnt;
    }
    if (client_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

int client_command_pass(t_client_info* client_info, char* argument)
{
    if (strncasecmp(argument, "230", 3) == 0) {
        client_info->is_authenticated = true;
        char* p = argument;
        while (p != NULL && *p != '\n')
            ++p;
        ++p;
        if (client_info->username != NULL) {
            free(client_info->username);
            client_info->username = NULL;
        }
        client_info->username = malloc(sizeof(char) * strlen(p));
        sprintf(client_info->username, "%s", p + strlen(USERNAME_VALUE));
        *(--p) = '\r';
        *(++p) = '\n';
        *(p + 1) = 0;
        p = client_info->username;
        while (p != NULL && !(*p == '\r' && *(p + 1) == '\n'))
            ++p;
        *p = 0;
    }
    return (0);
}

int client_command_cwd(t_client_info* client_info, char* argument)
{
    if (strncasecmp(argument, "250", 3) == 0) {
        char* p = argument;
        while (p != NULL && *p != '\n')
            ++p;
        ++p;
        sprintf(client_info->cwd, "%s", p + strlen(CWD_VALUE));
        *(--p) = '\r';
        *(++p) = '\n';
        *(p + 1) = 0;
        p = client_info->cwd;
        while (p != NULL && !(*p == '\r' && *(p + 1) == '\n'))
            ++p;
        *p = 0;
    }
    return (0);
}

int client_command_cdup(t_client_info* client_info, char* argument)
{
    if (strncasecmp(argument, "250", 3) == 0) {
        char* p = argument;
        while (p != NULL && *p != '\n')
            ++p;
        ++p;
        sprintf(client_info->cwd, "%s", p + strlen(CWD_VALUE));
        *(--p) = '\r';
        *(++p) = '\n';
        *(p + 1) = 0;
        p = client_info->cwd;
        while (p != NULL && !(*p == '\r' && *(p + 1) == '\n'))
            ++p;
        *p = 0;
    }
    return (0);
}

int client_command_quit(t_client_info* client_info, char* argument)
{
    return (1);
}

int client_command_port(t_client_info* client_info, char* argument)
{
    if (!client_info->is_authenticated)
        return (0);
    if (client_info->mode == PASSIVE) {
        client_info->mode = ACTIVE;
        /* printf("%s", argument); */
        if ((strncmp("127,0,0,1", argument, 9)))
            return (0);
        close(client_info->dataListen.fd);
        char* pointer = argument;
        while (*pointer == ' ')
            ++pointer;
        int port = 0;
        char buf[1024] = { "\0" };
        int cnt = 0;
        cnt = sprintf(buf + cnt, "%s.", strtok(argument, ","));
        cnt = sprintf(buf + cnt, "%s.", strtok(NULL, ","));
        cnt = sprintf(buf + cnt, "%s.", strtok(NULL, ","));
        cnt = sprintf(buf + cnt, "%s", strtok(NULL, ","));
        port += 256 * (atoi(strtok(NULL, ",")));
        port += (atoi(strtok(NULL, ",")));
        /* printf("%d\n", port);
         * printf("%s\n", buf); */
        data_socket_open(&client_info->dataListen, buf, port);
    }
    return (0);
}

int client_command_pasv(
    t_client_info* client_info, char* argument __attribute__((__unused__)))
{
    char* pointer = argument;
    socket_close(client_info->dataListen.fd);
    /* get_socket_addr(&client_info->dataListen, "127.0.0.1", 12002); */

    int port = 0;
    int commaCnt = 0;
    while (*pointer != 0) {
        if (*pointer == ',')
            ++commaCnt;
        ++pointer;
        if (commaCnt == 4)
            break;
    }
    for (int i = 0, tmp = 0, weight = 256; i < 2; ++i) {
        while (*pointer != ',' && (*pointer <= '9' && *pointer >= '0')) {
            tmp *= 10;
            tmp += (*pointer - '0');
            ++pointer;
        }
        port += weight * tmp;
        tmp = 0;
        weight = 1;
        ++pointer;
    }

    client_info->data.addrlen = sizeof(struct sockaddr_in);
    client_info->data.addr.sin_family = AF_INET;
    client_info->data.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* printf("%d\n", port); */
    client_info->data.addr.sin_port = htons(port);
    client_info->mode = PASSIVE;

    return (0);
}

int client_command_retr(t_client_info* client_info, char* argument)
{
    (void)argument;
    if (client_remote_connection(client_info) == -1) {
        perror("Client to Server Conection Failed");
        return (-1);
    }
    client_info->data_in_use = true;
    int read_len = 0;
    char buf[4096];
    int fd;
    for (int i = 0; i < strlen(argument); ++i) {
        if (argument[i] == '\r' || argument[i] == '\n') {
            argument[i] = 0;
            break;
        }
    }
    if ((fd = open(argument, O_CREAT | O_WRONLY, 0644)) < 0) {
        perror("File Open failed");
    }
    while ((read_len = read(client_info->data.fd, buf, 4096)) > 0) {
        write(fd, buf, read_len);
    }
    close(fd);
    if (client_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

int client_command_stor(t_client_info* client_info, char* argument)
{

    (void)argument;
    if (client_remote_connection(client_info) == -1) {
        perror("Client to Server Conection Failed");
        return (-1);
    }
    client_info->data_in_use = true;
    int read_len = 0;
    char buf[4096];
    int fd;
    for (int i = 0; i < strlen(argument); ++i) {
        if (argument[i] == '\r' || argument[i] == '\n') {
            argument[i] = 0;
            break;
        }
    }
    if ((fd = open(argument, O_RDONLY, 0644)) < 0) {
        perror("File Open failed");
    }
    while ((read_len = read(fd, buf, 4096)) > 0) {
        write(client_info->data.fd, buf, read_len);
    }
    close(fd);
    if (client_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

int client_command_pwd(t_client_info* client_info, char* argument)
{
    return (0);
}

int client_command_help(t_client_info* client_info, char* argument)
{
    return (0);
}

int client_command_noop(t_client_info* client_info, char* argument)
{
    return (0);
}
