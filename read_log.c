#include <stdio.h>
#include <string.h>

#define BUZZ_SIZE 1024

int main(int argc, char **argv)
{
    char buff[BUZZ_SIZE];
    FILE *f = fopen("/media/sdcard/data_lisa_log.txt", "r");
    int ch;
    
    

	while ( ( ch = fgetc(f) ) != EOF ) {
		
		if(ch == 0x99)
			printf("\n");
		printf("%d ",ch);
	}
    fclose(f);
    return 0;
}
