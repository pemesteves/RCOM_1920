#include <stdio.h>

#include "url.h"

int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("download should have 2 parameters: %s ftp://[<user>:<password>@]<host>/<url-path>\n\n", argv[0]);
        return -1;
    }

    URL url;

    if(parseURL(argv[1], &url)){
        return -1;
    }

    return 0;
}