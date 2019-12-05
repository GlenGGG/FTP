#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "client_commands.h"
#include "connect_to_server.h"
#include "socket.h"
#define MAX_BUF 4096

char lastCommand[MAX_BUF];

static void* read_from_server(void* arg)
{
    t_client_info* client_info = (t_client_info*)arg;
    char receive_buf[MAX_BUF];
    char next_buf[MAX_BUF];
    int read_len = 0;
    while ((read_len = read(client_info->server.fd, receive_buf, MAX_BUF - 1))
        > 0) {
        receive_buf[read_len] = 0;
        char* print_pointer = receive_buf;
        char* next_pointer = print_pointer;
        while (print_pointer) {
            int rb_len = strlen(print_pointer);
            while (!(*(next_pointer) == '\r' && *(next_pointer + 1) == '\n'))
                ++next_pointer;
            next_pointer += 2;
            if (next_pointer - print_pointer >= rb_len)
                next_pointer = NULL;
            if (next_pointer != NULL) {
                sprintf(next_buf, "%s", next_pointer);
                *next_pointer = 0;
                next_pointer = next_buf;
            }
            if (strncasecmp(
                    print_pointer, S_RESPONSE_PASS, strlen(S_RESPONSE_PASS))
                == 0) {
                print_pointer += strlen(S_RESPONSE_PASS);
                client_command_pass(client_info, print_pointer);
            } else if (strncasecmp(print_pointer, S_RESPONSE_PASV,
                           strlen(S_RESPONSE_PASV))
                == 0) {
                print_pointer += strlen(S_RESPONSE_PASV);
                client_command_pasv(client_info, print_pointer);
            } else if (strncasecmp(print_pointer, S_RESPONSE_PORT,
                           strlen(S_RESPONSE_PORT))
                == 0) {
                print_pointer += strlen(S_RESPONSE_PORT);
                client_command_port(client_info, lastCommand);
            } else if (strncasecmp(print_pointer, S_RESPONSE_CWD,
                           strlen(S_RESPONSE_CWD))
                == 0) {
                print_pointer += strlen(S_RESPONSE_CWD);
                client_command_cwd(client_info, print_pointer);
            } else if (strncasecmp(print_pointer, S_RESPONSE_CDUP,
                           strlen(S_RESPONSE_CDUP))
                == 0) {
                print_pointer += strlen(S_RESPONSE_CDUP);
                client_command_cdup(client_info, print_pointer);
            }
            rb_len = strlen(print_pointer);
            pthread_mutex_lock(&client_info->print_mutex);
            printf("%s", print_pointer);
            pthread_mutex_unlock(&client_info->print_mutex);
            if (print_pointer[rb_len - 1] == '\n'
                && print_pointer[rb_len - 2] == '\r') {
                pthread_mutex_lock(&client_info->go_on_mutex);
                client_info->go_on = true;
                pthread_cond_signal(&client_info->cond);
                pthread_mutex_unlock(&client_info->go_on_mutex);
            }
            if (next_pointer != NULL) {
                sprintf(receive_buf, "%s", next_buf);
                next_pointer = receive_buf;
            }
            print_pointer = next_pointer;
        }
    }
    return (0);
}

int connect_to_server(t_client_info* client_info, char* cwd)
{
    client_info->client.addrlen = sizeof(struct sockaddr_in);
    client_info->mode = ACTIVE;

    if ((connect(client_info->server.fd,
            (struct sockaddr*)&(client_info->server.addr),
            client_info->server.addrlen))
        < 0)
        perror(ECONNECT);
    getsockname(client_info->server.fd,
        (struct sockaddr*)&client_info->client.addr,
        &client_info->client.addrlen);
    data_socket_open(&client_info->dataListen, "127.0.0.1",
        ntohs(client_info->client.addr.sin_port));
    /* data_socket_open(&client_info->dataListen, "127.0.0.1",
     *     ntohs(client_info->client.addr.sin_port)); */
    pthread_mutex_init(&client_info->print_mutex, NULL);
    pthread_mutex_init(&client_info->go_on_mutex, NULL);
    pthread_cond_init(&client_info->cond, NULL);
    memset(lastCommand, 0, sizeof(lastCommand));

    client_info->go_on = false;
    pthread_t pthread_id;
    pthread_create(&pthread_id, NULL, read_from_server, (void*)client_info);
    usleep(5000);
    char command_buf[MAX_BUF << 2];
    do {
        int write_len = 0;
        pthread_mutex_lock(&client_info->go_on_mutex);
        while (!client_info->go_on) {
            pthread_cond_wait(&client_info->cond, &client_info->go_on_mutex);
        }
        printf("%s:%s$",
            client_info->username == NULL ? "NotLogin" : client_info->username,
            client_info->cwd[0] == 0 ? "/" : client_info->cwd);
        fgets(command_buf, MAX_BUF, stdin);
        while ((write_len += write(client_info->server.fd,
                    command_buf + write_len, strlen(command_buf) - write_len)),
            write_len < strlen(command_buf))
            ;
        client_info->go_on = false;
        pthread_mutex_unlock(&client_info->go_on_mutex);
        if (strncasecmp(command_buf, "list", 4) == 0) {
            if (client_info->is_authenticated == true)
                client_command_list(client_info, command_buf);
        } else if (strncasecmp(command_buf, "retr", 4) == 0) {
            if (client_info->is_authenticated == true)
                client_command_retr(client_info, command_buf + 5);
        } else if (strncasecmp(command_buf, "stor", 4) == 0) {
            if (client_info->is_authenticated == true)
                client_command_stor(client_info, command_buf + 5);
        } else if (strncasecmp(command_buf, "port", 4) == 0) {
            sprintf(lastCommand, "%s", command_buf + 5);
        }
    } while (1);
    return (0);
}
