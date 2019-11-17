#include <string.h>
#include <stdint.h>

char *compress_chunk(char *buf, int32_t buf_len, int32_t *outlen, char is_last)
{
    printf("Compressing chunk len=%i\n", buf_len);
    *outlen = buf_len + 5;
    char *out = (char *)malloc(*outlen);
    out[0] = is_last ? 1 : 0;
    int16_t len = *(&(buf_len));

    *(int16_t *)(out + 1) = len;
    *(int16_t *)(out + 3) = 0xffff - len;

    memcpy(out + 5, buf, len);

    return out;
};