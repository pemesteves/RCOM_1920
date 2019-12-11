#include <stdio.h>
#include <string.h>

#include "ftp.h"

int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("download should have 2 parameters: %s ftp://[<user>:<password>@]<host>/<url-path>\n\n", argv[0]);
        return -1;
    }

    URL url;
    if(parseURL(argv[1], &url)){
        return -1;
    }

    FTP ftp;
    ftp.url = url;
    char* buffer = malloc(2014 * sizeof(char));

    printf("\n# CONNECTING\n");
    if(ftp_connect_server(&ftp) < 0)
        exit(1);

    printf("\n# AUTHENTICATING USER\n");
    printf("\tUser: %s\n", url.user);
    printf("\tPass: %s\n\n", url.password);
    if(url.user != "") {
        if(ftp_user_command(&ftp, buffer))
            exit(2);
        bzero(buffer, strlen(buffer));
        if(ftp_pass_command(&ftp, buffer))
            exit(3);
    }

    printf("\n# ENTERING PASSIVE MODE\n");
    if(ftp_pasv_command(&ftp, buffer))
        exit(4);

    printf("\n# CHANGING DIRECTORY\n");
    if(ftp_cwd_command(&ftp, buffer))
        exit(5);

    printf("\n# DOWNLOADING FILE\n");
    if(ftp_retr_command(&ftp, buffer))
        exit(6);

    ftp_close_server(&ftp);

    free(buffer);
    return 0;
}