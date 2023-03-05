#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    FILE *fp = fopen("/dev/llkd_miscdrv", "rw+b");
    
    if (fp == NULL)
    {
        fputs("could not open /dev/llkd_miscdrv\n", stderr);
        exit(1);
    }

    char buf[8192] = { '1' };

    fread(buf, sizeof buf, 1, fp);

    printf("old secret: %s\n", buf);

    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    char *a = asctime(timeinfo);

    strncpy(buf, a, strlen(a) + 1);
    fwrite(buf, sizeof buf, 1, fp);
    fread(buf, sizeof buf, 1, fp);
    printf("and now it is: %s\n", buf);

    fclose(fp);

    return 0;
}
