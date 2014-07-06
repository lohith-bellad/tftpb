/*----------------------------------------------------------------------------
 Filename:-    tftpb.h
 Description:- This is the header file for Trivial File Transfer Protocol(TFTP)
			implementation. The implementation has been done based on 
			RFC 1035, by Sollins, MIT July, 1992.
 Date:- 	     June 29th 2014
 Author:- 	Lohith S Bellad
               University of Southern California, Information Science Institute
 Platform:-    Mac OS X Mountain Lion
 Place:-       Los Angeles, California.
----------------------------------------------------------------------------*/

/* header files for system calls and functions based on BSD 4.4 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <dirent.h>
#include <ctype.h>


/* Opcodes for TFTP implementation */
#define RRQ	01
#define WRQ 	02
#define DATA 	03
#define ACK 	04
#define ERR 	05

/* Other usefull constant declarations */
#define DEF_DATA_SIZE	512
#define TIMEOUT		3000		
#define MAX_RETXN		3
#define MAXDATA		1024

/* Error messages */
char err_0[40] = "Not Defined!!!";
char err_1[40] = "File not found";
char err_2[40] = "Access violation/Permission denied";
char err_3[40] = "Disk Full/Allocation exceeded";
char err_4[40] = "Illegal TFTP operation";
char err_5[40] = "Unknown transfer ID";
char err_6[40] = "File already exists";
 

