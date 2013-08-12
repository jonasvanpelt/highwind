#include <stdio.h>
#include <string.h>

#define BUZZ_SIZE 1024

int main(int argc, char **argv)
{
    char buff[BUZZ_SIZE];
    FILE *f = fopen("/media/sdcard/data_lisa_log.txt", "r");
    fgets(buff, BUZZ_SIZE, f);
    printf("String read: %s\n", buff);
    fclose(f);
    return 0;
}
