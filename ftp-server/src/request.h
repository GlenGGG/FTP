#ifndef _REQUEST_H
#define _REQUEST_H

#include <stdio.h>

# define REQUEST_DELIMITER ' '
# define REQUEST_DELIMITER_PTR " "
# define REQUEST_L "\n"
# define REQUEST_END "\r\n"
# define REQUEST_END_L "\n"

# define DB_USERNAME "anonymous"
# define DB_PASS ""

# define REQUEST_RESPONSE(fd, format, ...) \
      dprintf(fd, format""REQUEST_END, ##__VA_ARGS__),\
      printf(format""REQUEST_END, ##__VA_ARGS__)\

# define REQUEST_RESPONSE_S(fd, format, ...) \
  dprintf(fd, format, ##__VA_ARGS__),\
  printf(format, ##__VA_ARGS__)

# define REQUEST_RESPONSE_L(fd, format, ...) \
      dprintf(fd, format""REQUEST_L, ##__VA_ARGS__),\
      printf(format""REQUEST_L, ##__VA_ARGS__)

# define SINGLE_STRING "%s"


char	**request_parse_arguments(char *argument);
char	**request_parse_arguments_delimiter(char *argument, char delimiter);
void	request_free_arguments(char **arguments);

int	request_count_arguments(char **arguments);

# define SERVER_HELP_FULL "Server recognize these commands:"

/*
** reply codes
*/
# define SERVER_150 "150 File status okay; about to open data connection."

# define SERVER_200 "200 Command okay."
# define SERVER_000 "000 Command okay."
# define SERVER_211 "211 System status, or system help reply."
# define SERVER_214 "214 Help message."
# define SERVER_220 "220 (myFTP, v1.0)"
# define SERVER_221 "221 Service closing control connection."
# define SERVER_226 "226 Closing data connection."
# define SERVER_227 "227 Entering Passive Mode (%s,%s,%s,%s,%d,%d)."
# define SERVER_230 "230 User logged in, proceed."
# define SERVER_250 "250 Requested file action okay, completed."
# define SERVER_257 "257 \"%s\" created."

# define SERVER_331 "331 User name okay, need password."

# define SERVER_421 "421 Service not available, closing control connection."
# define SERVER_425 "425 Can't open data connection."
# define SERVER_450 "450 Requested file unavailable."

# define SERVER_500 "500 Syntax error, command unrecognized."
# define SERVER_502 "502 Command not implemented."
# define SERVER_503 "503 Bad sequence of commands."
# define SERVER_530 "530 Not logged in."
# define SERVER_550 "550 Requested action not taken."


#endif
