CC=gcc
CFLAGS=-I.

all: download

download: download.o url.o
	$(CC) -o download download.o url.o

clean:
	rm -frf *.o *~ download 