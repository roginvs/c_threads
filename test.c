#include "./crc32.test.c"

/*
gcc -o /tmp/test test.c && /tmp/test
*/

int main()
{
    int crc32_test_result = crc32_test();
    if (crc32_test_result)
    {
        return crc32_test_result;
    };

    printf("All tests done\n");
}
