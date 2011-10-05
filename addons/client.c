#define NP1 "/tmp/n1"
#define NP2 "/tmp/n2"
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int wrfd = 0;
    int rd = 0;
    wrfd = open(NP1, O_WRONLY);
    rd = open(NP2, O_RDONLY);

    char str[100] = "";
    while(1) {
        printf("\ncrafty$ ");
        scanf("%s", str);
        str[strlen(str)]='\n';
        write(wrfd, str, strlen(str));
    }
}
