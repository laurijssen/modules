#include <stdio.h>

int main()
{
    FILE *file = fopen("/sys/kernel/debug/tasker/processes", "r");
    if (!file)
    {
        perror("Failed to open debugfs file");
        return 1;
    }

    char buffer[4096];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    fclose(file);

    return 0;
}