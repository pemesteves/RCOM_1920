CC=gcc
CFLAGS=-I.

all: emissor recetor

recetor: recetor.o link_layer.o app_layer.o files.o
	$(CC) -o recetor recetor.o link_layer.o app_layer.o files.o

emissor: emissor.o link_layer.o app_layer.o files.o
	$(CC) -o emissor emissor.o link_layer.o app_layer.o files.o

clean:
	rm -frf *.o *~ emissor recetor 