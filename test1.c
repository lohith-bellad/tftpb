#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	FILE *fp;
	char buff[512];
	int n,fd;
	
	if( (fp = fopen("quotes.txt","r")) == NULL)
	{
		printf("Error\n");
		return 0;
	}
	fd = fileno(fp);
	memset(buff,0,512);
	if( (n = read(fd,buff,511)) < 0)
	{
		printf("Error\n");
		return 0;
	}
	printf("%d\n",n);
	printf("%s",buff);
	
	memset(buff,0,512);
	if( (n = read(fd,buff,511)) < 0)
	{
		printf("Error\n");
		return 0;
	}
	printf("%d\n",n);
	printf("%s",buff);
	/*
	memset(buff,0,512);
	if( (n = fread(buff,511,1,fp)) < 0)
	{
		printf("Error\n");
		return 0;
	}
	printf("%s\n",buff);
	return 0;*/
}