// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/* June 29th 2014, Lohith S Bellad
 * University of Southern California, Information Science Institute
 * Los Angeles, California.
 */

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
#define RRQ	1
#define WRQ 	2
#define DATA 	3
#define ACK 	4
#define ERR 	5

/* Other usefull constant declarations */
#define DEF_DATA_SIZE	1024
#define TIMEOUT		3000		
#define MAX_RETXN	3
#define MAXDATA		1024

/* Error messages */
char err[][40] = { "Not Defined!!!",
                "File not found",
                "Access violation/Permission denied",
                "Disk Full/Allocation exceeded",
                "Illegal TFTP operation",
                "Unknown transfer ID",
                "File already exists"};
 
/* Server side variables */
char def_path[128] = "/Users/lohith_bellad/Desktop/tftp_files/";
unsigned char ack_buf[32];
unsigned char recv_data_buf[2048];

/* error handling */
void err_sys(int flag, char *buf)
{
	if(flag == 1)
	{
		perror(buf);
		exit(0);
	}
	printf("Error: %s\n",buf);
	exit(0);
}
