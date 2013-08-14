#include <stdio.h>
#include <string.h>
#include <stdio.h>

#define BUZZ_SIZE 1024

int main(int argc, char **argv)
{
    char buff[BUZZ_SIZE];
    FILE *f = fopen("/media/sdcard/data_lisa_log.txt", "r");
    int ch;
    int startBitFound=0;
    int length;
    int i;
    
    while(ch = fgetc(f) != EOF){
		//read until we find start byte 0x99
		while ( ch != 0x99 ) {
			ch = fgetc(f);
			usleep(1000);
		}

		buff[0]=ch;
		length = fgetc(f);
		buff[1]=length;

		for(i=2;i<length;i++){
			buff[i]=fgetc(f);
		}
		
		//print buf
		for(i=0;i<length;i++){
			printf("%d ",buff[i]);
		}
		printf("\n\n");
		

		/*while ( ( ch = fgetc(f) ) != EOF ) {

		if(ch == 0x99)
		printf("\n\n");
		printf("%d ",ch);
		}*/
	}
    
    
	
    fclose(f);
    return 0;
}
