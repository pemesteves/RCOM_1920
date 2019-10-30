#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "files.h"

int file_exist(char* file_name){
    struct stat st;   

    return stat (file_name, &st);
}

int get_file_size(applicationLayerFile *file){
    struct stat st;
    if(stat(file->file_name, &st) < 0)
        return -1;
    
    file->file_size = st.st_size;
    return 0;
}

int open_file(applicationLayerFile *file){//char* file_name){
    int fd = open(file->file_name, O_RDONLY);
    if(fd < 0){
        perror("open");
        return -1;
    }
    file->fd = fd;
    return 0;
}


int read_file(int fd, unsigned char* string, int length){
    int num_read = 0;

    if((num_read = read(fd, string, length)) < 0){
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

int create_file(char* file_name){
    int fd;
    if((fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0750)) < 0){
        perror("open file\n");
        return -1;
    }

    return fd;
}

int write_file(int fd, unsigned char* content, unsigned int content_size){
    if(write(fd, content, content_size) < 0){
        perror("write");
        return -1;
    }
    return 0;
}



