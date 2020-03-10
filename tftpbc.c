/*
 * Copyright (c) 2019, Lohith Bellad
 * Copyright (c) 2014, Lohith Bellad
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Client side implementation for Trivial File Transfer Protocol(TFTP).
 * The implementation is based on RFC 1350, by Sollins, MIT July, 1992.
 *
 * June 30th 2014.
 *
 * Lohith Bellad
 * USC Information Sciences Institute, Marina Del Rey, California.
 *
 * Usage:
 * ./tftpbc [-s] server [-h] [-u] | [-d] filename [-l] length 
 * [-s] -- server info
 * [-h] -- help menu(this one)
 * [-l] -- length of the data packet(next version, def = 1024 bytes)
 * [-u] -- upload the file (specify the filemane)
 * [-d] -- download the file (specify the filename)
 *
 */

#include "tftpb.h"

int sock_id;
struct sockaddr_in server_addr;
int TID,server_addr_len;
struct timeval tyme1,tyme2;
unsigned long int time1,time2;
double rate;

/* function to form req packet */
int build_req_pkt(int opcode, char *file, char *buf);

/* function to start data transmission */
unsigned int start_data_tx(char *filename);

/* function to form error packet */
int build_err_pkt(int err_no, char *buf);

/* function to start data reception */
void start_data_rx(char *buf, char *filename);

static void
display_help() {

	printf("Usage:\n");
	printf("./tftpbc [-s] server [-h] [-u] | [-d] filename [-l]"
	       "length\n"); 
	printf("[-s] -- server info\n");
	printf("[-h] -- help menu(this one)\n");
	printf("[-l] -- length of the data packet (next version,"
	       "def = 1024 bytes)\n");
	printf("[-u] -- upload the file (specify the filename)\n");
	printf("[-d] -- download the file (specify the filename)\n");
}

int 
main (int argc, char **argv) {

	struct hostent *host_ip = NULL;
	struct in_addr *addr;
	extern char *optarg;
	int speed_flag = 0, err_no;
	char filename[128];
	unsigned int data_txd;
	int c;
	int opcode=0;
	ssize_t sent_data;
	int n,len,i;
	unsigned char *ptr;
	FILE *fp;
	
	pkt_data_len = DEF_DATA_SIZE;
	if (argc == 1) {
		printf("Please enter the server and service info\n");
		display_help();
		exit(1);
	}

	while ((c = getopt(argc,argv,"h:s:l:d:u:r")) != -1) {
		switch (c) {
			
		case 'h':
		display_help();
		return 0;

		case 'r':
		speed_flag = 1;
		break;

		case 's':
		if((host_ip = gethostbyname(optarg)) == NULL) {
			printf("Error in finding server IP address\n");
			exit(1);
		}
		break;

		case 'u':
		struct stat st;
		opcode = RRQ;
		strncpy(filename, optarg, sizeof(filename));
		if (stat(filename, &st) != 0) {
			printf("File does not exist\n");
			exit(1);
		}
		break;

		case 'd':
		opcode = RRQ;
		strncpy(filename, optarg, sizeof(filename));
		fp = fopen(filename, "w");
		if(fp == NULL) {
			printf("File cannot be created in the recieving"
			       "machine\n");
			exit(1);
		}
		fclose(fp);
		break;

		case 'l':
		pkt_data_len = min(atoi(optarg), 1350);
		if(pkt_data_len != DEF_DATA_SIZE)
			printf("Packet data length changed from default"
			       "value to %d\n",pkt_data_len);
		break;

		default:
		display_help();
		exit(0);
		}
	}

	if ((host_ip == NULL) || (opcode == 0)) {
		printf("Please specify the server and service type\n");
		display_help();
		exit(1);
	}

	addr = (struct in_addr *)host_ip->h_addr;
	printf("Connecting to %s...\n",inet_ntoa(*addr));
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr. sin_addr, addr, host_ip->h_length);
	server_addr.sin_port = htons(69);

	if ((sock_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error creating the socket, errno = %d\n", errno);
		exit(0);
	}

	if (fnctl(sock_id, F_SETFL, O_NONBLOCK) != 0) {
		printf("Failed to set socket to non-blocking mode\n");
		exit(0);
	}
	
	len = send_req_pkt(opcode, filename);

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
	for(i=0; i < 40; i++)
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
	else if(*ptr == 5)
	{
		err_no = recv_buf[3];
		printf("%s",err[err_no]);
		return 0;
	}
	else if(*ptr++ == 3)
	{
		printf("Transfering data, please wait...\n");
		TID = ntohs(server_addr.sin_port);
		start_data_rx(ptr,filename);
	}
	else
	{
		printf("Connection failed...:(\n");
		return 0;
	}
	ptr++;
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
	/* main function ends */
}  

int build_req_pkt(int opcode, char *file, char *buf)
{
	int len = 0;
	if(opcode == RRQ)
	{
		if( (len = sprintf(buf, "%c%c%s%c%s%c", 0x01,0x01, file,0x00, "netascii", 0x00)) < 0)
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
	return len;gg
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

int build_ack_pkt(int block_cnt, unsigned char *buf)
{
	int len;
	if( (len = sprintf((char*)buf,"%c%c%c%c",0x00,0x04,0x00,0x00)) < 0)
	{
		printf("Error in forming the Ack packet\n");
		return 0;
	}
	buf[2] = (block_cnt & 0xFF00) >> 8;
	buf[3] = block_cnt & 0x00FF; 
	return len;
}

unsigned int start_data_tx(char *filename)
{
	int n, sent_data, i,resend_cnt,k, fd;
	int err_pkt_len=0;
	unsigned char data_pkt[1500];
	unsigned char *ptr;
	FILE *fp;
	int last_pkt=0;
	unsigned int data_txd = 0;
	unsigned int block_cnt;
	
	if( (fp = fopen(filename,"r")) == NULL)
	{
		printf("Error opening file for reading\n");
		return 0;
	}
	fd = fileno(fp); /* file pointer to file descriptor conversion */
	/* block_cnt can go upto 65535!!! */
	for(block_cnt = 1; block_cnt < 65535; block_cnt++)
	{
		memset(data_buf,0,sizeof(data_buf));
		if( (n = read(fd,data_buf,pkt_data_len)) < 0)
		{
			printf("Error reading data from file\n");
			exit(0);
		}
		data_txd += n;
		if(block_cnt == 60)
			write(1,"#",1);
		if(block_cnt % 60 == 0)
			write(1,"\n",1);
		if(n < pkt_data_len)
		{
			last_pkt = 1;
			write(1,"#...done\n",9);
		}
		else
			write(1,"#",1);
		
		n = sprintf((char *)data_pkt,"%c%c%c%u%s",0x00,0x03,0x00,0x00,data_buf);
		data_pkt[2] = (block_cnt & 0xFF00) >> 8;
		data_pkt[3] = block_cnt & 0x00FF;
		
		for(resend_cnt = 0; resend_cnt < 4; resend_cnt++)
		{
			if(resend_cnt != 0)
				write(1,"*",1);
			sent_data = sendto(sock_id,(void*)data_pkt,n,0,(const struct sockaddr *)&server_addr,sizeof(server_addr));
			if(sent_data != n)
			{
				printf("Error in sending the data\n");
				exit(0);
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
						fclose(fp);
						exit(0);
					}
					continue;
				}
				
				ptr = recv_buf;
				if((*ptr++ != 0 || *ptr++ !=4 ) || (recv_buf[2] != (block_cnt & 0xFF00) >> 8) || (recv_buf[3] != (block_cnt & 0x00FF)) )
					continue;
				else
					break;
			}
		}
		if(k < 0)
		{
			printf("\nConnection timedout!!!\n");
			fclose(fp);
			exit(0);
		}
		if(last_pkt == 1)
			break;
	}
	fclose(fp);
	return data_txd;
}

void start_data_rx(char *buf, char *filename)
{
	FILE *fp;
	int fd,ack_len,n,bb_cnt,i;
	int sent_data;
	unsigned char *ptr;
	char ack_buf[8];
	char recv_buf[1500];
	/* opening file for writing */
	if( (fp = fopen(filename, "w")) == NULL)
	{
		printf("Error opening file\n");
		return;
	}
	fd = fileno(fp); /* file pointer to file descriptor conversion */
	if( (write(fd,buf,strlen(buf))) < 0)
	{
		printf("Error writing to file\n");
	}
	
	ack_len = build_ack_pkt(bb_cnt, ack_buf);
	if( (sent_data = sendto(sock_id,(void*)ack_buf,ack_len,0,(const struct sockaddr *)&server_addr,sizeof(server_addr))) != len)
	{
		printf("Error in sending the ack packet\n");
		return;
	}
	printf("Ack sent successfully\n");
	
	n=0;
	memset(recv_buf,0,sizeof(recv_data_buf));
	for(i=0; i< 60; i++)
	{
		if( (n = recvfrom(sock_id,&recv_buf,sizeof(recv_buf),MSG_DONTWAIT,NULL,NULL)) < 0)
			usleep(250000);
		else
			break;
	}
	if(n < 0)
	{
		printf("\n:(\nConnection Timedout...\n\n");
		exit(0);
	}
	ptr = recv_buf;
	
	if(*ptr++ != 0 || *ptr++ != 3)
	{
		printf("Unrecognised packet\n");
		return;
	}
	//extracting block count 
	bb_cnt = recv_data_buf[2] << 8;
	bb_cnt = bb_cnt | recv_data_buf[3];
	ptr = &(recv_data_buf[4]);
	
}
