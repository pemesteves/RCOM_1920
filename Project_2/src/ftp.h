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

#include "url.h"

// Server replies
#define FILE_OK          150
#define DATA_OPEN        125
#define FILE_ACTION_OK   250
#define COMMAND_OK       200
#define SERVER_BUSY      120
#define SERVER_READY     220
#define CLOSE_CONNECTION 226
#define USER_LOGGED      230
#define DIRECTORY_OK     257
#define PASSWORD_NEEDED  331

typedef struct {
    int socket_fd;
    int data_socket_fd;
    URL url;
} FTP;

int ftp_connect_server(FTP* ftp);

void ftp_close_server(FTP* ftp);

int ftp_server_reply(FTP* ftp, char* reply);

int ftp_command(FTP* ftp, const char* cmd, const char* arg, char* reply);

int ftp_usr_command(FTP* ftp, char* reply);

int ftp_pass_command(FTP* ftp, char* reply);

int ftp_cwd_command(FTP* ftp, char* reply);

int ftp_pasv_command(FTP* ftp, char* reply);

int ftp_retr_command(FTP* ftp, char* reply);