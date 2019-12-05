#ifndef _COMMANDS_H
# define _COMMANDS_H

# include "socket.h"

#define S_RESPONSE_PASV "S_RESPONSE_PASV "
#define S_RESPONSE_PASS "S_RESPONSE_PASS "
#define S_RESPONSE_PORT "S_RESPONSE_PORT "
#define S_RESPONSE_CWD "S_RESPONSE_CWD "
#define S_RESPONSE_CDUP "S_RESPONSE_CDUP "

#define USERNAME_VALUE "username: "
#define CWD_VALUE "cwd: "

typedef	struct s_command_alias {
    char *title;
    char *help;
    int	(*exec)(t_client_info *client_info, char *argument);
} t_command_alias;

extern t_command_alias	server_commands[35];

int	server_remote_connection(t_client_info *client_info);
char	*server_get_real_path(char *path, char *root);
int	server_command_port_init_socket(t_client_info *client_info, char **infos);

int server_command_unimplemented(t_client_info* client_info, char* argument);
int	server_command_user(t_client_info *client_info, char *argument);
# define HELP_USER "USER <SP> <username> <CRLF>"
int	server_command_pass(t_client_info *client_info, char *argument);
# define HELP_PASS "PASS <SP> <password> <CRLF>"
int	server_command_acct(t_client_info *client_info, char *argument);
# define HELP_ACCT "ACCT <SP> <account-information> <CRLF>"
int	server_command_cwd(t_client_info *client_info, char *argument);
# define HELP_CWD "CWD  <SP> <pathname> <CRLF>"
int	server_command_cdup(t_client_info *client_info, char *argument);
# define HELP_CDUP "CDUP <CRLF>"
int	server_command_smnt(t_client_info *client_info, char *argument);
# define HELP_SMNT "SMNT <SP> <pathname> <CRLF>"
int	server_command_rein(t_client_info *client_info, char *argument);
# define HELP_REIN "REIN <CRLF>"
int	server_command_quit(t_client_info *client_info, char *argument);
# define HELP_QUIT "QUIT <CRLF>"
int	server_command_port(t_client_info *client_info, char *argument);
# define HELP_PORT "PORT <SP> <host-port> <CRLF>"
int	server_command_pasv(t_client_info *client_info, char *argument);
# define HELP_PASV "PASV <CRLF>"
int	server_command_type(t_client_info *client_info, char *argument);
# define HELP_TYPE "TYPE <SP> <type-code> <CRLF>"
int	server_command_stru(t_client_info *client_info, char *argument);
# define HELP_STRU "STRU <SP> <structure-code> <CRLF>"
int	server_command_mode(t_client_info *client_info, char *argument);
# define HELP_MODE "MODE <SP> <mode-code> <CRLF>"
int	server_command_retr(t_client_info *client_info, char *argument);
# define HELP_RETR "RETR <SP> <pathname> <CRLF>"
int	server_command_stor(t_client_info *client_info, char *argument);
# define HELP_STOR "STOR <SP> <pathname> <CRLF>"
int	server_command_stou(t_client_info *client_info, char *argument);
# define HELP_STOU "STOU <CRLF>"
int	server_command_appe(t_client_info *client_info, char *argument);
# define HELP_APPE "APPE <SP> <pathname> <CRLF>"
int	server_command_allo(t_client_info *client_info, char *argument);
# define HELP_ALLO "ALLO <SP> <decimal-integer>"
int	server_command_rest(t_client_info *client_info, char *argument);
# define HELP_REST "REST <SP> <marker> <CRLF>"
int	server_command_rnfr(t_client_info *client_info, char *argument);
# define HELP_RNFR "RNFR <SP> <pathname> <CRLF>"
int	server_command_rnto(t_client_info *client_info, char *argument);
# define HELP_RNTO "RNTO <SP> <pathname> <CRLF>"
int	server_command_abor(t_client_info *client_info, char *argument);
# define HELP_ABOR "ABOR <CRLF>"
int	server_command_dele(t_client_info *client_info, char *argument);
# define HELP_DELE "DELE <SP> <pathname> <CRLF>"
int	server_command_rmd(t_client_info *client_info, char *argument);
# define HELP_RMD "RMD  <SP> <pathname> <CRLF>"
int	server_command_mkd(t_client_info *client_info, char *argument);
# define HELP_MDK "MKD  <SP> <pathname> <CRLF>"
int	server_command_pwd(t_client_info *client_info, char *argument);
# define HELP_PWD "PWD  <CRLF>"
int	server_command_list(t_client_info *client_info, char *argument);
# define HELP_LIST "LIST [<SP> <pathname>] <CRLF>"
int	server_command_nlst(t_client_info *client_info, char *argument);
# define HELP_NLST "NLST [<SP> <pathname>] <CRLF>"
int	server_command_site(t_client_info *client_info, char *argument);
# define HELP_SITE "SITE <SP> <string> <CRLF>"
int	server_command_syst(t_client_info *client_info, char *argument);
# define HELP_SYST "SYST <CRLF>"
int	server_command_stat(t_client_info *client_info, char *argument);
# define HELP_STAT "STAT [<SP> <pathname>] <CRLF>"
int	server_command_help(t_client_info *client_info, char *argument);
# define HELP_HELP "HELP [<SP> <string>] <CRLF>"
int	server_command_noop(t_client_info *client_info, char *argument);
# define HELP_NOOP "NOOP <CRLF>"

#endif
