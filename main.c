#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "./gzip.c"
/*

gcc -o main.out main.c -lpthread && ./main.out test/hpmor_ru.html /tmp/a && (cat /tmp/a | xxd)

gcc -o main.out main.c -lpthread && ./main.out /tmp/kek /tmp/kek.gz && (cat /tmp/kek.gz | xxd)


gcc -o main.out main.c -lpthread && ./main.out test/hpmor_ru.html /tmp/a && (cat /tmp/a | gzip -d > 11111)

*/

void writer(char *buf, int32_t len, void *user_data)
{
    printf("Writer is called len=%i\n", len);
    fwrite(buf, len, 1, (FILE *)user_data);
}

int main(int argc, char *argv[])
{
    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    int32_t threads_count = 4;

    printf("Opening input file %s\n", input_file_name);
    int fd = open(input_file_name, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening input file");
        exit(1);
    }
    struct stat sb;
    fstat(fd, &sb);
    __off_t input_buf_size = sb.st_size;
    printf("Size: %lu\n", input_buf_size);

    unsigned char *input_buf = NULL;
    if (input_buf_size != 0)
    {
        input_buf = mmap(NULL, input_buf_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (input_buf == MAP_FAILED)
        {
            perror("Error with mmap");
            exit(1);
        }
    };

    printf("Opening output file %s\n", output_file_name);
    FILE *p_output_file = fopen(output_file_name, "wb");
    gzip(input_buf, input_buf_size, threads_count, &writer, p_output_file);
    fclose(p_output_file);

    if (input_buf_size != 0)
    {
        if (munmap(input_buf, input_buf_size) == -1)
        {
            perror("Error un-mmapping input file");
        }
    }
    close(fd);
}