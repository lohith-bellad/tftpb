/*----------------------------------------------------------------------------
 Filename:-    tftpbs.c
 Description:- This is the server implementation c file for Trivial File 
			Transfer Protocol(TFTP)implementation. The implementation 
			has been done based on RFC 1035, by Sollins, MIT July, 1992.
 Date:- 	     July 9th, 2014
 Author:- 	Lohith S Bellad
               University of Southern California, Information Sciences Institute
 Platform:-    Mac OS X Mountain Lion
 Place:-       Los Angeles, California.
----------------------------------------------------------------------------*/

#include "tftpb.h"

/* Global variables */
struct sockaddr_in serv_addr,client_addr;
socklen_t clilen;

/* function to form ack packets */
int build_ack_pkt(int block_cnt, char *buf);

/* function to kill zombies */
void sig_handler(int sig);

/* function to recv packets */
void start_recv(int sock_id, char *filename);

/* function to form error packet */
int build_err_pkt(int err_no, char *buf);

/* main function */
int main(int argc, char **argv)
{
	int sock_id,n;
	char buf[512];
	char *ptr,filename[256];
	
	printf("------------------------------------------------------------\n");
	printf("|        Trivial File Transfer Protocol (TFTP)             |\n");
	printf("------------------------------------------------------------\n");
	signal(SIGCHLD, sig_handler);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(69);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	
	if( (sock_id = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
	{
		printf("Error creating the sever udp port\n");
		return 0;
	}
	
	if( bind(sock_id,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	{
		printf("Bind failed, errno=%d\n",errno);
		return 0;
	}
	clilen = (socklen_t)sizeof(client_addr);
	write(1,"TFTP server is listening...\n",28);
	
	while(1)
	{
		printf("socket id=%d\n",sock_id);
		if( (n = recvfrom(sock_id,&buf,sizeof(buf),0,(struct sockaddr *)&client_addr,&clilen)) < 0)
		{
			printf("Error in receiving, errno = %d, %d\n",n,errno);
			return 0;
		}
		if( (fork() == 0)) /* child process */
		{
			close(sock_id);
			if( (sock_id = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
			{
				printf("Error creating the socket\n");
				return 0;
			}
			ptr = buf;
			if(*ptr++ != 0)
			{
				printf("Unrecognised packet\n");
				return 0;
			}
			if(*ptr == RRQ)
			{
				/* handled differently */
			}
			else if(*ptr == WRQ)
			{
				strcpy(filename,ptr);
				start_recv(sock_id,filename);
			}
		}
	}
}

int build_ack_pkt(int block_cnt, char *buf)
{
	int len;
	if( (len = sprintf(buf,"%c%c%c%c",0x00,0x04,0x00,0x00)) < 0)
	{
		printf("Error in forming the Ack packet\n");
		return 0;
	}
	buf[2] = block_cnt & 0xFF00;
	buf[3] = block_cnt & 0x00FF; 
	return len;
}

void sig_handler(int sig)
{
	pid_t pid;
	int stat;
	pid = wait(&stat);
	printf("Child with pid %d killed with status=%d\n",pid,stat);
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

void start_recv(int sock_id, char *filename)
{
	int err_pkt_len;
	FILE *fp;
	int i,fd,n,sent_data,bb_cnt;
	char *ptr;
	int len,last_pkt=0;
	
	strcat(def_path,filename);
	if( (fp = fopen(def_path,"w")) == NULL)
	{
		err_pkt_len = build_err_pkt(2,err_pkt);
		if( (sent_data = sendto(sock_id,(void*)err_pkt,err_pkt_len,0,(const struct sockaddr *)&client_addr,sizeof(client_addr))) != err_pkt_len)
		{
			printf("Error sending error packet to peer\n");
			return;
		}
		close(sock_id);
		return;
	}
	fclose(fp);
	memset(ack_buf,0,sizeof(ack_buf));
	len = build_ack_pkt(0, ack_buf);
	if( (sent_data = sendto(sock_id,(void*)ack_buf,len,0,(const struct sockaddr *)&client_addr,sizeof(client_addr))) != len)
	{
		printf("Error in sending the ack packet\n");
		return;
	}
	printf("Ack sent successfully\n");
	
	while(1)
	{
		n=0;bb_cnt=0;
		memset(recv_data_buf,0,sizeof(recv_data_buf));
		for(i=0; i< 40; i++)
		{
			if( (n = recvfrom(sock_id,&recv_data_buf,sizeof(recv_data_buf),MSG_DONTWAIT,NULL,NULL)) < 0)
				usleep(250000);
			else
				break;
		}
		if(n < 0)
		{
			printf("\n:(\nConnection Timedout...\n\n");
			return;
		}
		ptr = recv_data_buf;
		
		if(*ptr++ != 0 || *ptr++ != 3)
		{
			printf("Unrecognised packet\n");
			return;
		}
		//extracting block count 
		bb_cnt = recv_data_buf[2] << 16;
		bb_cnt = recv_data_buf[3];
		printf("block_cnt = %d\n",bb_cnt);
		ptr = &recv_data_buf[4];
		if(strlen(ptr) < 512)
			last_pkt = 1;
		if( (fp = fopen(def_path,"a")) == NULL)
		{
			printf("File cannot be created in server\n"); /* XXX */
			return;
		}
		fd = fileno(fp); /* file pointer to fd conversion */
		
		memset(send_buf,0,sizeof(ack_buf));
		len = build_ack_pkt(bb_cnt, ack_buf);
		if( (sent_data = sendto(sock_id,(void*)ack_buf,len,0,(const struct sockaddr *)&client_addr,sizeof(client_addr))) != len)
		{
			printf("Error in sending the request packet\n");
			return;
		}
		if( (n = write(fd,ptr,512)) != 512)
		{
			printf("Error writing data to file\n");
			return;
		}
		if(last_pkt == 1)
			break;
	}
	fclose(fp);
	close(sock_id);
	return;
}