#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>

// Server replies
#define FILE_OK          "150"
#define COMMAND_OK       "200"
#define SERVER_READY     "220"
#define CLOSE_CONNECTION "226"
#define USER_LOGGED      "230"
#define DIRECTORY_OK     "257"
#define PASSWORD_NEEDED  "331"

typedef struct {
    int socket_fd;
} FTP;

int ftp_connect_server(FTP* ftp, const char* ip, int port);

int ftp_server_reply(FTP* ftp, char** reply);