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
#include <sys/time.h>
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
#include <sys/time.h>
#include <time.h>
#include <signal.h>


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

/* constant buffers */
char data_buf[513];

/* Error messages */
char err[][40] ={ "Not Defined!!!",
                "File not found",
                "Access violation/Permission denied",
                "Disk Full/Allocation exceeded",
                "Illegal TFTP operation",
                "Unknown transfer ID",
                "File already exists"};
 
/* Client side varaibles */
char send_buf[256];
char recv_buf[516];
char err_pkt[64];

/* Server side variables */
char def_path[128] = "/Users/lohith_bellad/Desktop/tftp_files/";
char ack_buf[32];
char recv_data_buf[520];