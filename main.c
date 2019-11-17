#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "./gzip.c"
/*

gcc -o main.out main.c -lpthread && ./main.out test/hpmor_ru.html /tmp/aaaaa

*/

void writer(char *buf, int len, void *user_data)
{
    printf("Writer is called len=%i\n", len);
    fwrite(buf, len, 1, (FILE *)user_data);
}

int main(int argc, char *argv[])
{
    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    int threads_count = 4;

    printf("Opening input file %s\n", input_file_name);
    int fd = open(input_file_name, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening input file");
        exit(1);
    }
    struct stat sb;
    fstat(fd, &sb);
    printf("Size: %lu\n", sb.st_size);

    unsigned char *memblock = NULL;
    if (sb.st_size != 0)
    {
        memblock = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (memblock == MAP_FAILED)
        {
            perror("Error with mmap");
            exit(1);
        }
    };

    gzip(memblock, sb.st_size, 4, &writer, NULL);

    if (sb.st_size != 0)
    {
        if (munmap(memblock, sb.st_size) == -1)
        {
            perror("Error un-mmapping input file");
        }
    }
    close(fd);
}