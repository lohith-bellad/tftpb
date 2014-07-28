#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
int len;
char buf[64];

if( (len = sprintf(buf,"%c%c%s",0x54,0x55,"Welcome")) < 0)
{
	printf("Error\n");
	return 0;
}

printf("%c\n",buf[0]);

}