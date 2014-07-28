/*----------------------------------------------------------------------------
 Filename:-    tftpbc.c
 Description:- This is the client implementation c file for Trivial File 
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
   ./tftpbc [-s] server [-h] [-u] | [-d] filename [-l] length 
   [-s] -- server info
   [-h] -- help menu(this one)
   [-l] -- length of the data packet
   [-u] -- upload the file (specify the filemane)
   [-d] -- download the file (specify the filename)
*/

/* Global variables */
int sock_id;
struct sockaddr_in server_addr;
int TID,server_addr_len;
struct timeval tyme1,tyme2;
unsigned long int time1,time2;
double rate;
/* function to display the help */
void display_help();

/* function to form req packet */
int build_req_pkt(int opcode, char *file, char *buf);

/* function to start data transmission */
unsigned int start_data_tx(char *filename);

/* function to form error packet */
int build_err_pkt(int err_no, char *buf);

/* main function */
int main (int argc, char **argv)
{
	struct hostent *host_ip = NULL;
	struct in_addr *addr;
	extern char *optarg;
	int speed_flag=0;
	char filename[128];
	unsigned int data_txd;
	
	int c;
	int pkt_data_len,opcode=0;
	ssize_t sent_data;
	int n,len,i;
	char *ptr;
	FILE *fp;
	
	/* some display of info, so that main has started */
	printf("------------------------------------------------------------\n");
	printf("|        Trivial File Transfer Protocol (TFTP)             |\n");
	printf("------------------------------------------------------------\n");
	
	/* setting default values */
	pkt_data_len = DEF_DATA_SIZE;
	if(argc == 1)
	{
		printf("Please enter the server and service info\n");
		display_help();
		return 0;
	}

	/* parsing the command line options */
	while( (c = getopt(argc,argv,"h:s:l:d:u:r")) != -1)
	{
		switch(c)
		{
			/* display usage and quit */
			case 'h':
			display_help();
			return 0;
		     
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
			opcode = WRQ;
			strncpy(filename,optarg,sizeof(filename));
			fp = fopen(filename,"r");
			if(fp == NULL)
			{
				printf("File does not exist\n");
				return 0;
			}
			fclose(fp);
			//printf("Filename taken: %s\n",filename);
			break;
			
			/* save filename to be downloaded */
			case 'd':
			opcode = RRQ;
			strncpy(filename,optarg,strlen(filename));
			fp = fopen(filename, "w");
			if(fp == NULL)
			{
				printf("File cannot be created in the recieving machine\n");
				return 0;
				break;
			}
			fclose(fp);
			//printf("Filename taken: %s\n",filename);
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
	
	/* clear the server address structure */
	memset(&server_addr,0,sizeof(server_addr));
	/* fill the server sock addr structure */
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr,addr, host_ip->h_length);
	server_addr.sin_port = htons(69); /* tftp well known port number, only used at the initial connection setup */
	
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
	sent_data = sendto(sock_id,(void*)send_buf,len,0,(const struct sockaddr *)&server_addr,sizeof(server_addr));
	if(sent_data < 0)
	{
		printf("Error in sending the request packet, errno = %d\n",errno);
		return 0;
	}
	memset(&recv_buf,0,sizeof(recv_buf));
	memset(&server_addr,0,sizeof(server_addr));
	n=0;
	server_addr_len = sizeof(server_addr);
	for(i=0; i < 20; i++)
	{
		if( (n = recvfrom(sock_id,&recv_buf,sizeof(recv_buf),MSG_DONTWAIT,(struct sockaddr*)&server_addr,(socklen_t *)&server_addr_len)) < 0)
			usleep(250000);	
		else
			break;
	}
	if(n < 0)
	{
		printf("\n:(\nSeems like TFTP Server is down, Please try after some time... \n\n");
		return 0;
	}
	ptr = recv_buf;
	if(*ptr++ != 0)
	{
		printf("Unrecognised packet\n");
		return 0;
	}
	if(*ptr == 4)
	{
		printf("Transfering data, please wait...\n");
		TID = ntohs(server_addr.sin_port);
	}
	else
	{
		printf("Connection failed...:(\n");
		return 0;
	}
	
	if(opcode == WRQ)
	{
		if(gettimeofday(&tyme1, NULL) < 0)
			printf("Error in calculating the data rate\n");
		data_txd = start_data_tx(filename);
		if(gettimeofday(&tyme2, NULL) < 0)
			printf("Error in calculating the data rate\n");
		time1 = tyme1.tv_usec + (tyme1.tv_sec * 1000000);
		time2 = tyme2.tv_usec + (tyme2.tv_sec * 1000000);
		time1 = time2 - time1;
		rate = (data_txd * 1000.0 * 8)/ time1;
		if(speed_flag == 1)
			printf("%d bytes transferred in %5.2fsecs, data rate = %5.2fkbps\n\n",data_txd,(time1/1000000.0),rate);
	}
	else
		printf("Need to be done\n");
	/* main function ends */
}  

int build_req_pkt(int opcode, char *file, char *buf)
{
	int len = 0;
	if(opcode == RRQ)
	{
		if( (len = sprintf(buf, "%c%c%s%c%s%c", 0x01,0x01, file, 0x00, "netascii", 0x00)) < 0)
		{
			printf("Error in forming the request packet\n");
			return 0;
		}
	}
	else
	{
		if( (len = sprintf(buf, "%c%c%s%c%s%c", 0x00,0x02, file, 0x00, "netascii", 0x00)) < 0)
		{
			printf("Error in forming the request packet\n");
			return 0;
		}
	}
	return len;
}

void display_help()
{
	printf("Usage:\n");
     printf("./tftpbc [-s] server [-h] [-u] | [-d] filename [-l] length\n"); 
	printf("[-s] -- server info\n");
     printf("[-h] -- help menu(this one)\n");
     printf("[-l] -- length of the data packet\n");
     printf("[-u] -- upload the file (specify the filename)\n");
     printf("[-d] -- download the file (specify the filename)\n");
	printf("------------------------Thank you---------------------------\n");
}

int build_err_pkt(int err_no,char *buf)
{
	int len=0;
	switch(err_no)
	{
		case 0:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x00,err[errno],0x00);
		break;
		case 1:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x01,err[errno],0x00);
		break;
		case 2:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x02,err[errno],0x00);
		break;
		case 3:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x03,err[errno],0x00);
		break;
		case 4:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x04,err[errno],0x00);
		break;
		case 5:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x05,err[errno],0x00);
		break;
		case 6:
		len = sprintf(buf, "%c%c%c%c%s%c",0x00,0x05,0x00,0x06,err[errno],0x00);
		break;
	}	
	if(len < 0)
	{
		printf("Error forming the error packet\n");
		return 0;
	}
	return len;
}

unsigned int start_data_tx(char *filename)
{
	int n, sent_data, i,resend_cnt,k, fd;
	int err_pkt_len=0;
	char data_pkt[517],*ptr;
	FILE *fp;
	int block_cnt,last_pkt=0;
	unsigned int data_txd = 0;
	
	if( (fp = fopen(filename,"r")) == NULL)
	{
		printf("Error opening file for reading\n");
		return 0;
	}
	fd = fileno(fp); /* file pointer to file descriptor conversion */
	/* block_cnt can go upto 65535!!! */
	for(block_cnt = 1; block_cnt < 65535; block_cnt++)
	{
		memset(data_buf,0,513);
		if( (n = read(fd,data_buf,512)) < 0)
		{
			printf("Error reading data from file\n");
			return 0;
		}
		data_txd += n;
		if(n < 512)
		{
			last_pkt = 1;
			write(1,"#...done\n",9);
		}
		else
			write(1,"#",1);
		
		n = sprintf(data_pkt,"%c%c%c%c%s",0x00,0x03,0x00,0x00,data_buf);
		data_pkt[2] = block_cnt & 0xFF00;
		data_pkt[3] = block_cnt & 0x00FF;
		
		for(resend_cnt = 0; resend_cnt < 3; resend_cnt++)
		{
			sent_data = sendto(sock_id,(void*)data_pkt,n,0,(const struct sockaddr *)&server_addr,sizeof(server_addr));
			if(sent_data != n)
			{
				printf("Error in sending the data\n");
				return 0;
			}
			k=0;
			server_addr_len = sizeof(server_addr);
			for(i=0; i < 40; i++)
			{
				if( (k = recvfrom(sock_id,&recv_buf,sizeof(recv_buf),MSG_DONTWAIT,(struct sockaddr*)&server_addr,(socklen_t *)&server_addr_len)) < 0)
					usleep(250000);	
				else
					break;
			}
			if(k > 0)
			{
				if(ntohs(server_addr.sin_port) != TID)
				{
					err_pkt_len = build_err_pkt(5,err_pkt);
					if( (sent_data = sendto(sock_id,(void*)err_pkt,err_pkt_len,0,(const struct sockaddr *)&server_addr,sizeof(server_addr))) != err_pkt_len)
					{
						printf("Error sending error packet to Anonymous\n");
						return 0;
					}
					continue;
				}
				
				ptr = recv_buf;
				if(*ptr++ != 0 || *ptr++ !=4 || (*ptr++ != (block_cnt & 0xFF00)) || (*ptr != (block_cnt & 0x00FF)))
					continue;
				else
					break;
			}
		}
		if(k < 0)
		{
			printf("Connection timedout!!!\n");
			return 0;
		}
		if(last_pkt == 1)
			break;
	}
	return data_txd;
}

