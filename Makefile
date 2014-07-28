#Author: Lohith S Bellad, Information Science Institute
#        Marina Del Ray, California	

CC = gcc
CFLAGS = -Wall

all: tftpbc tftpbs
	
tftpbc: tftpbc.c
	$(CC) $(CFLAGS) -g -o tftpbc tftpbc.c
	
tftpbs: tftpbs.c
	$(CC) $(CFLAGS) -g -o tftpbs tftpbs.c
	
clean: 
	rm -f tftpbc tftpbs