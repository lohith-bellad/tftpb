/*----------------------------------------------------------------------------
 Filename:-    tftpbc.c
 Description:- This is the client implemntation c file for Trivial File 
			Transfer Protocol(TFTP)implementation. The implementation 
			has been done based on RFC 1035, by Sollins, MIT July, 1992.
 Date:- 	     June 30th 2014
 Author:- 	Lohith S Bellad
               University of Southern California, Information Sciences Institute
 Platform:-    Mac OS X Mountain Lion
 Place:-       Los Angeles, California.
----------------------------------------------------------------------------*/

#include "tftpb.h"

/* Usage:
   ./tftpbc [-s] server [-h] [-u] | [-d] filename [-r] [-l] length 
   [-s] -- server info
   [-h] -- help menu(this one)
   [-r] -- display the speed of tranfer
   [-l] -- length of the data packet
   [-u] -- upload the file (specify the filemane)
   [-d] -- download the file (specify the filename)
*/

/* function to display the help */
void display_help();

/* function to form req packet */
int build_req_pkt(int opcode, char *file, char *buf);

/* main function */
int main (int argc, char **argv)
{
	struct hostent *host_ip = NULL;
	struct in_addr *addr;
	extern char *optarg;
	int speed_flag=0;
	int c,sock_id;
	int pkt_data_len,opcode=0;
	char filename[128];
	struct sockaddr_in server_addr;
	char send_buf[256];
	ssize_t sent_data;
	int len;
	FILE *fp;
	/* some display of info, so that main has started */
	printf("------------------------------------------------------------\n");
	printf("|        Trivial File Transfer Protocol (TFTP)             |\n");
	printf("------------------------------------------------------------\n");
	
	/* setting default values */
	pkt_data_len = 512;
	
	if(argc == 1)
	{
		printf("Please enter the server and service info\n");
		display_help();
		return 0;
	}
	opterr = 0;
	/* parsing the command line options */
	while( (c = getopt(argc,argv,"h:s:l:d:u:r")) != -1)
	{
		switch(c)
		{
			/* display usage and quit */
			case 'h':
			display_help();
			return 0;
			break;
		     
			/* set the speed display flag */
			case 'r':
			speed_flag = 1;
			break;
			
			/*obtaining the server IP */
			case 's':
			if( (host_ip = gethostbyname(optarg)) == NULL)
			{
				printf("Error in finding server IP address\n");
				return 0;
			}
			break;
			
			/* save filename to be uploaded */
			case 'u':
			opcode = 2;
			strncpy(filename,optarg,strlen(filename));
			fp = fopen(filename,"r");
			if(fp == NULL)
			{
				printf("File does not exist\n");
				return 0;
				break;
			}
			fclose(fp);
			printf("Filename taken: %s\n",filename);
			break;
			
			/* save filename to be downloaded */
			case 'd':
			opcode = 1;
			strncpy(filename,optarg,strlen(filename));
			fp = fopen(filename, "w");
			if(fp == NULL)
			{
				printf("File cannot be created in the recieving machine\n");
				return 0;
				break;
			}
			fclose(fp);
			printf("Filename taken: %s\n",filename);
			break;
			
			/* set packet size for data transaction */
			case 'l':
			pkt_data_len = atoi(optarg);
			if(pkt_data_len != 512)
				printf("Packet data length changed from default value to %d\n",pkt_data_len);
			break;
			
			default:
			display_help();
			return 0;
		}
	}
	/* checking for the right and appropriate number of command line arguments */
	if(host_ip == NULL)
	{
		printf("Please specify the server info\n");
		display_help();
		return 0;
	}
	if(opcode == 0)
	{
		printf("Please specify the service type\n");
		display_help();
		return 0;
	}
	
	/* display some info to user */
	addr = (struct in_addr *)host_ip->h_addr;
	printf("Connecting to %s...\n",inet_ntoa(*addr));
	
	/* fill the server sock addr structure */
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr,addr, host_ip->h_length);
	server_addr.sin_port = htons(69); /* tftp well known port number, only used at the initial setup */
	
	/* creating the socket id */
	if( (sock_id = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1)
	{
		printf("Error creating the socket, errno = %d\n",errno);
		return 0;
	}
	/* clearing the send buffer */
	memset(send_buf,0,sizeof(send_buf));
	/* creating the first packet */
	len = build_req_pkt(opcode,filename,send_buf);
	sent_data = sendto(sock_id,(void*)send_buf,strlen(send_buf),0,(const struct sockaddr *)&server_addr,sizeof(server_addr));
	if(sent_data < 0)
	{
		printf("Error in sending the request packet\n");
		return 0;
	}
	if(sent_data == len)
		printf("Request packet sent successfully\n");
	else
	{
		printf("Error in sending request packet\n");
		return 0;
	}
}  

int build_req_pkt(int opcode, char *file, char *buf)
{
	int len;
	if( (len = sprintf(buf,"%c%c%s%c%s%c",'0','1',file,'0',"netascii",'0')) < 0)
	{
		printf("Error in forming the request packet\n");
		return 0;
	}
	return len;
}
void display_help()
{
	printf("Usage:\n");
     printf("./tftpbc [-s] server [-h] [-u] | [-d] filename [-l] length [-r]\n"); 
	printf("[-s] -- server info\n");
     printf("[-h] -- help menu(this one)\n");
     printf("[-r] -- display the speed of tranfer\n");
     printf("[-l] -- length of the data packet\n");
     printf("[-u] -- upload the file (specify the filename)\n");
     printf("[-d] -- download the file (specify the filename)\n");
	printf("------------------------Thank you---------------------------\n");
}