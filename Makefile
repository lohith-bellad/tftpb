CC = gcc
CFLAGS = -Wall

all: tftpbc
	
tftpbc: tftpbc.c
	$(CC) $(CFLAGS) -o tftpbc tftpbc.c
	
clean: 
	rm -f tftpbc