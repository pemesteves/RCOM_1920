#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

typedef char content[256];

typedef struct {
    content user;
    content password;
    content host;
    content path;
    content ip;
    int port;
} URL;

int parseURL(char* url, URL *parsed_url);