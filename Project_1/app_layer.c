#include "app_layer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

int open_file(char* file_name){
    int fd = open(file_name, O_RDONLY);
    if(fd < 0){
        perror("open");
        return -1;
    }

    return 0;
}


int read_file(int fd, unsigned char* string){
    int num_read = 0;
    if((num_read = read(fd, string, sizeof(string)/sizeof(string[0]))) < 0){
        perror("read");
        return -1;
    }

    return num_read;
}

int close_file(int fd){
    if(close(fd) < 0){
        perror("close");
        return -1;
    }
    return 0;
}