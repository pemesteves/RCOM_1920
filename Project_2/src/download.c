#include <stdio.h>

int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("download should have 2 parameters: %s ftp://ftp.up.pt/pub/...\n\n", argv[0]);
        return -1;
    }

    return 0;
}