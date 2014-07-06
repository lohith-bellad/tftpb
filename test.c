	#include <ctype.h>
     #include <stdio.h>
     #include <stdlib.h>
     #include <unistd.h>
     
     int
     main (int argc, char **argv)
     {
       int aflag = 0;
       int bflag = 0;
       char *cvalue = NULL;
       int index;
       int c;
     
       opterr = 0;
     
  	while( (c = getopt(argc,argv,"hs:l:u:d:")) != -1)
  	{
  		printf("%d\n",c);
  		switch(c)
  		{
  			case 'h':
			printf("pressed h\n");
  			return 0;
  			break;
		     
  			case 's':
  			aflag = 1;
  			break;
			
  			case 'u':
  			
  			break;
			
  			case 'd':
  			
  			break;
			
  			case 'l':
  			
  			break;
			
  			default:
  			
  			return 0;
  		}
  	}
       return 0;
     }