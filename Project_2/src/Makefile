CC=gcc
CFLAGS=-I.

all: download

download: download.o url.o ftp.o
	$(CC) -o download download.o url.o ftp.o 

clean:
	rm -frf *.o *~ download 