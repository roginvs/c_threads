#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "./gzip.c";
/*

gcc -o main.out main.c -lpthread && ./main.out

*/

void writer(char *buf, int len, void *user_data)
{
    printf("Writer is called len=%i\n", len);
    // TODO: Implement writing
    usleep(1000000);
}
int main(int argc, char *argv[])
{
    int threads_count = 4;

    // TODO
}