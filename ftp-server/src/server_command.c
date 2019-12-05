#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "server_command.h"
#include "socket.h"

t_command_alias server_commands[35]
    = { { "USER", HELP_USER, &server_command_user },
          { "PASS", HELP_PASS, &server_command_pass },
          { "ACCT", HELP_ACCT, &server_command_unimplemented },
          { "CWD", HELP_CWD, &server_command_cwd },
          { "CDUP", HELP_CDUP, &server_command_cdup },
          { "SMNT", HELP_SMNT, &server_command_unimplemented },
          { "REIN", HELP_REIN, &server_command_unimplemented },
          { "QUIT", HELP_QUIT, &server_command_quit },
          { "PORT", HELP_PORT, &server_command_port },
          { "PASV", HELP_PASV, &server_command_pasv },
          { "TYPE", HELP_TYPE, &server_command_unimplemented },
          { "STRU", HELP_STRU, &server_command_unimplemented },
          { "MODE", HELP_MODE, &server_command_unimplemented },
          { "RETR", HELP_RETR, &server_command_retr },
          { "STOR", HELP_STOR, &server_command_stor },
          { "STOU", HELP_STOU, &server_command_unimplemented },
          { "APPE", HELP_APPE, &server_command_unimplemented },
          { "ALLO", HELP_ALLO, &server_command_unimplemented },
          { "REST", HELP_REST, &server_command_unimplemented },
          { "RNFR", HELP_RNFR, &server_command_unimplemented },
          { "RNTO", HELP_RNTO, &server_command_unimplemented },
          { "ABOR", HELP_ABOR, &server_command_unimplemented },
          { "DELE", HELP_DELE, &server_command_unimplemented },
          { "RMD", HELP_RMD, &server_command_unimplemented },
          { "MDK", HELP_MDK, &server_command_unimplemented },
          { "PWD", HELP_PWD, &server_command_pwd },
          { "LIST", HELP_LIST, &server_command_list },
          { "NLST", HELP_NLST, &server_command_unimplemented },
          { "SITE", HELP_SITE, &server_command_unimplemented },
          { "SYST", HELP_SYST, &server_command_unimplemented },
          { "STAT", HELP_STAT, &server_command_unimplemented },
          { "HELP", HELP_HELP, &server_command_help },
          { "?", HELP_HELP, &server_command_help },
          { "NOOP", HELP_NOOP, &server_command_noop }, { 0 } };
static int server_command_list_close(t_client_info* client_info)
{
    if (client_info->mode == DEFAULT)
        return (0);
    if (socket_close(client_info->data.fd) == -1)
        return (-1);
    return (0);
}

int server_command_unimplemented(t_client_info* client_info, char* argument)
{
    if (!client_info->is_authenticated) {
        REQUEST_RESPONSE(client_info->client.fd, SERVER_530);
        return (0);
    }
    REQUEST_RESPONSE(client_info->client.fd, SERVER_502);
    (void)argument;
    return (0);
}

int server_remote_connection(t_client_info* client_info)
{
    client_info->data.addrlen = sizeof(struct sockaddr);
    if (client_info->mode == PASSIVE) {
        if ((client_info->data.fd = accept(client_info->dataListen.fd,
                 (struct sockaddr*)&client_info->data.addr,
                 &client_info->data.addrlen))
            == -1)
            return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    } else if (client_info->mode == ACTIVE) {
        usleep(50000);
        struct protoent* pe;
        if (!(pe = getprotobyname(PROTOTYPE_NAME))
            || (client_info->data.fd = socket(client_info->data.addr.sin_family,
                    SOCK_STREAM, pe->p_proto))
                == -1)
            return (-1);
        if (client_info->mode == ACTIVE
            && connect(client_info->data.fd,
                   (struct sockaddr*)&client_info->data.addr,
                   client_info->data.addrlen)
                == -1)
            return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    }
    return (0);
}

char* server_get_real_path(char* path, char* root)
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

int server_command_user(t_client_info* client_info, char* argument)
{
    char** arguments;

    if (client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (!(arguments = request_parse_arguments(argument)))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (request_count_arguments(arguments) == 0)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (client_info->username)
        free(client_info->username);
    if (!(client_info->username = strdup(arguments[0])))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    REQUEST_RESPONSE(client_info->client.fd, SERVER_331);
    request_free_arguments(arguments);
    return (0);
}

static int server_command_pass_fake_db_call(char* username, char* password)
{
    if (strcasecmp(username, DB_USERNAME) != 0)
        return (1);
    return (0);
}

int server_command_pass(t_client_info* client_info, char* argument)
{
    char** arguments;

    if (!client_info->is_authenticated && !client_info->username)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_503), 0);
    if (client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_230), 0);
    if (!(arguments = request_parse_arguments(argument)))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (!server_command_pass_fake_db_call(client_info->username, arguments[0]))
        client_info->is_authenticated = true;
    if (client_info->is_authenticated)
        REQUEST_RESPONSE(client_info->client.fd,
            S_RESPONSE_PASS "" SERVER_230 "\n" USERNAME_VALUE "%s",
            client_info->username);
    else
        REQUEST_RESPONSE(client_info->client.fd, SERVER_530);
    request_free_arguments(arguments);
    return (0);
}

int server_command_cwd(t_client_info* client_info, char* argument)
{
    char** arguments;

    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (!(arguments = request_parse_arguments(argument)))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (request_count_arguments(arguments) == 0)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_550), 0);
    strcat(
        arguments[0][0] != '/' ? client_info->cwd : client_info->root_dir, "/");
    strcat(arguments[0][0] != '/' ? client_info->cwd : client_info->root_dir,
        arguments[0]);
    if (chdir(client_info->cwd) == -1)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_550), 0);
    if (getcwd(client_info->cwd, sizeof(client_info->cwd)) == NULL)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (strncmp(client_info->root_dir, client_info->cwd,
            strlen(client_info->root_dir))
        != 0)
        chdir(client_info->root_dir);
    if (getcwd(client_info->cwd, sizeof(client_info->cwd)) == NULL)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    REQUEST_RESPONSE(client_info->client.fd,
        S_RESPONSE_CWD "%s\n" CWD_VALUE "%s", SERVER_250,
        client_info->cwd + strlen(client_info->root_dir) - 1);
    request_free_arguments(arguments);
    return (0);
}

int server_command_cdup(t_client_info* client_info, char* argument)
{
    (void)argument;
    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (chdir("..") == -1)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_550), 0);
    if (getcwd(client_info->cwd, sizeof(client_info->cwd)) == NULL)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (strncmp(client_info->root_dir, client_info->cwd,
            strlen(client_info->root_dir))
        != 0)
        chdir(client_info->root_dir);
    if (getcwd(client_info->cwd, sizeof(client_info->cwd)) == NULL)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    REQUEST_RESPONSE(client_info->client.fd, SERVER_250 "\n" CWD_VALUE "%s",
        client_info->cwd + strlen(client_info->root_dir) - 1);
    return (0);
}

int server_command_quit(t_client_info* client_info, char* argument)
{
    (void)argument;
    REQUEST_RESPONSE(client_info->client.fd, SERVER_221);
    return (1);
}

int server_command_port_init_socket(t_client_info* client_info, char** infos)
{
    socket_close(client_info->dataListen.fd);
    char ip[16];
    int port;
    int ret;
    /* struct protoent *pe; */

    if (request_count_arguments(infos) != 6)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_500), 0);
    ret = sprintf(ip, "%s.%s.%s.%s", infos[0], infos[1], infos[2], infos[3]);
    ip[ret] = 0;
    port = atoi(infos[4]) * 256 + atoi(infos[5]);
    client_info->data.addrlen = sizeof(struct sockaddr_in);
    client_info->data.addr.sin_family = AF_INET;
    client_info->data.addr.sin_addr.s_addr = inet_addr(ip);
    client_info->data.addr.sin_port = htons(port);
    client_info->mode = ACTIVE;
    return (
        REQUEST_RESPONSE(client_info->client.fd, S_RESPONSE_PORT "" SERVER_200),
        0);
}

static int server_command_port_close(t_client_info* client_info)
{
    if (client_info->mode == DEFAULT)
        return (0);
    /* if (socket_close(client_info->data.fd) == -1)
     *   return (-1); */
    if (client_info->data_in_use && socket_close(client_info->data.fd))
        return (-1);
    return (0);
}

int server_command_port(t_client_info* client_info, char* argument)
{
    char** arguments;
    char** infos;

    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (server_command_port_close(client_info) == -1)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (!(arguments = request_parse_arguments(argument)))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (request_count_arguments(arguments) == 0)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_500), 0);
    if (!(infos = request_parse_arguments_delimiter(arguments[0], ',')))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    request_free_arguments(arguments);
    if (server_command_port_init_socket(client_info, infos) == -1)
        return (-1);
    request_free_arguments(infos);
    return (0);
}

static int server_command_pasv_init_server(t_client_info* client_info)
{
    socket_close(client_info->dataListen.fd);
    if (socket_open(&client_info->dataListen, 0) == -1
        || getsockname(client_info->dataListen.fd,
               (struct sockaddr*)&client_info->dataListen.addr,
               &client_info->dataListen.addrlen)
            == -1)
        return (-1);
    return (0);
}

static int server_command_pasv_close(t_client_info* client_info)
{
    if (client_info->mode == DEFAULT)
        return (0);
    /* if (socket_close(client_info->dataListen.fd) == -1)
     *   return (-1); */
    if (client_info->data_in_use && socket_close(client_info->data.fd))
        return (-1);
    return (0);
}

int server_command_pasv(
    t_client_info* client_info, char* argument __attribute__((__unused__)))
{
    char* ip;
    int dataListen_port;
    char* ip_t[4];

    client_info->dataListen.addrlen = sizeof(struct sockaddr);
    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (server_command_pasv_close(client_info) == -1)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (server_command_pasv_init_server(client_info) == -1)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if ((ip = socket_get_ip(client_info->server)) == NULL)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    dataListen_port = ntohs(client_info->dataListen.addr.sin_port);
    ip_t[0] = strtok(ip, ".");
    ip_t[1] = strtok(NULL, ".");
    ip_t[2] = strtok(NULL, ".");
    ip_t[3] = strtok(NULL, ".");
    REQUEST_RESPONSE(client_info->client.fd, S_RESPONSE_PASV "" SERVER_227,
        ip_t[0], ip_t[1], ip_t[2], ip_t[3], dataListen_port / 256,
        dataListen_port % 256);
    client_info->mode = PASSIVE;
    free(ip);
    return (0);
}

int server_command_retr(t_client_info* client_info, char* argument)
{
    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (client_info->mode == DEFAULT)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_425), 0);
    if (server_remote_connection(client_info) == -1) {
        perror("server remote connection in list");
        return (-1);
    }
    client_info->data_in_use = true;
    REQUEST_RESPONSE_L(client_info->client.fd, SERVER_150);
    int fd;
    char file_name_buf[8024];
    memset(file_name_buf, 0, sizeof(file_name_buf));
    sprintf(file_name_buf, "%s/%s", client_info->cwd, argument + 1);
    printf("file name: %s\ncwd: %s\n", argument + 1, client_info->cwd);
    for (int i = 0; i < strlen(file_name_buf); ++i) {
        if (file_name_buf[i] == '\r' || file_name_buf[i] == '\n') {
            file_name_buf[i] = 0;
            break;
        }
    }
    if ((fd = open(file_name_buf, O_RDONLY, 0644)) == -1) {
        perror("File open failed");
        if (server_command_list_close(client_info) == -1) {
            REQUEST_RESPONSE(client_info->client.fd, REQUEST_END);
            return (-1);
        }
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_450), 0);
    }
    int ret = 0;
    char buffer[40960];
    while ((ret = read(fd, buffer, 40960)) > 0) {
        (void)write(client_info->data.fd, buffer, ret);
    }
    close(fd);
    REQUEST_RESPONSE(client_info->client.fd, SERVER_226);
    if (server_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

int server_command_stor(t_client_info* client_info, char* argument)
{
    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (client_info->mode == DEFAULT)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_425), 0);
    if (server_remote_connection(client_info) == -1) {
        perror("server remote connection in list");
        return (-1);
    }
    client_info->data_in_use = true;
    REQUEST_RESPONSE_L(client_info->client.fd, SERVER_150);
    int fd;
    char file_name_buf[8024];
    memset(file_name_buf, 0, sizeof(file_name_buf));
    sprintf(file_name_buf, "%s/%s", client_info->cwd, argument + 1);
    printf("file name: %s\ncwd: %s\n", argument + 1, client_info->cwd);
    for (int i = 0; i < strlen(file_name_buf); ++i) {
        if (file_name_buf[i] == '\r' || file_name_buf[i] == '\n') {
            file_name_buf[i] = 0;
            break;
        }
    }
    if ((fd = open(file_name_buf, O_CREAT | O_WRONLY, 0644)) == -1) {
        perror("File open failed");
        if (server_command_list_close(client_info) == -1) {
            return (-1);
        }
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_450), 0);
    }
    int ret = 0;
    char buffer[40960];
    while ((ret = read(client_info->data.fd, buffer, 40960)) > 0) {
        (void)write(fd, buffer, ret);
    }
    close(fd);
    REQUEST_RESPONSE(client_info->client.fd, SERVER_226);
    if (server_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

int server_command_pwd(t_client_info* client_info, char* argument)
{
    (void)argument;
    if (!client_info->is_authenticated) {
        REQUEST_RESPONSE(client_info->client.fd, SERVER_530);
        return (0);
    }
    if (strcmp(client_info->root_dir, client_info->cwd) == 0)
        REQUEST_RESPONSE(client_info->client.fd, SERVER_257, "/");
    else
        REQUEST_RESPONSE(client_info->client.fd, SERVER_257,
            (client_info->cwd + strlen(client_info->root_dir) - 1));
    return (0);
}

int server_command_list_loop(t_client_info* client_info, char* argument)
{
    FILE* in;
    char stdout[4096];
    int i;

    (void)argument;
    i = 0;
    if (!(in = popen("ls -l", "r")))
        return (-1);
    while (fgets(&stdout[i], 4096 - i, in))
        i = strlen(stdout);
    pclose(in);
    REQUEST_RESPONSE(client_info->data.fd, SINGLE_STRING, stdout);
    return (0);
}

int server_command_list(t_client_info* client_info, char* argument)
{
    if (!client_info->is_authenticated)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_530), 0);
    if (client_info->mode == DEFAULT)
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_425), 0);
    if (server_remote_connection(client_info) == -1) {
        perror("server remote connection in list");
        return (-1);
    }
    client_info->data_in_use = true;
    REQUEST_RESPONSE_L(client_info->client.fd, SERVER_150);
    if (server_command_list_loop(client_info, argument) == -1) {
        REQUEST_RESPONSE(client_info->client.fd, REQUEST_END);
        return (-1);
    }
    usleep(5000);
    REQUEST_RESPONSE(client_info->client.fd, SERVER_226);
    if (server_command_list_close(client_info) == -1)
        return (-1);
    client_info->data_in_use = false;
    return (0);
}

static void server_command_help_full(t_client_info* client_info)
{
    int i;

    i = 0;
    REQUEST_RESPONSE_L(client_info->client.fd, SERVER_HELP_FULL);
    while (server_commands[i].title) {
        if (i != 0 && i != 10 && i != 20 && i != 30)
            REQUEST_RESPONSE_S(client_info->client.fd, SINGLE_STRING, " ");
        REQUEST_RESPONSE_S(
            client_info->client.fd, SINGLE_STRING, server_commands[i].title);
        if (i == 9 || i == 19 || i == 29)
            REQUEST_RESPONSE_S(client_info->client.fd, SINGLE_STRING, "\n");
        i++;
    }
    REQUEST_RESPONSE_S(client_info->client.fd, SINGLE_STRING, "\n");
    REQUEST_RESPONSE(client_info->client.fd, SERVER_214);
}

static void server_command_help_single(
    t_client_info* client_info, char* command)
{
    int i;

    i = 0;
    while (server_commands[i].title) {
        if (strcasecmp(server_commands[i].title, command) == 0) {
            REQUEST_RESPONSE_L(
                client_info->client.fd, SINGLE_STRING, server_commands[i].help);
            break;
        }
        i++;
    }
    if (server_commands[i].title)
        REQUEST_RESPONSE(client_info->client.fd, SERVER_211);
    else
        server_command_help_full(client_info);
}

int server_command_help(t_client_info* client_info, char* argument)
{
    char** arguments;

    if (!(arguments = request_parse_arguments(argument)))
        return (REQUEST_RESPONSE(client_info->client.fd, SERVER_421), -1);
    if (request_count_arguments(arguments) == 0)
        server_command_help_full(client_info);
    else
        server_command_help_single(client_info, arguments[0]);
    request_free_arguments(arguments);
    return (0);
}

int server_command_noop(t_client_info* client_info, char* argument)
{
    (void)argument;
    REQUEST_RESPONSE(client_info->client.fd, SERVER_200);
    return (0);
}
