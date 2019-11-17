#include <string.h>
#include <stdint.h>

/**
 *  TODO: Check len. Block with no compression are not more than 0xFFFF
 * 
 *  Output should be byte-aligned. Use empty "no compression" block in the end
 *   to achieve alignment
 * 
 * */
char *compress_chunk(char *buf, int32_t buf_len, int32_t *outlen, char is_last)
{
    printf("Compressing chunk len=%i\n", buf_len);

    /*
    int32_t gzip_blocks = buf_len / 0xffff + (buf_len % 0xffff == 0 ? 0 : 1);

    if (buf_len == 0)
    {
        *outlen = 0;
        return NULL;
    };

    *outlen = gzip_blocks * 5 + buf_len;
    char *out = (char *)malloc(*outlen);

    for (int32_t i = 0; i < gzip_blocks; i++)
    {
    }
*/

    *outlen = buf_len + 5;
    char *out = (char *)malloc(*outlen);
    out[0] = is_last ? 1 : 0;
    int16_t len = *(&(buf_len));

    *(int16_t *)(out + 1) = len;
    *(int16_t *)(out + 3) = 0xffff - len;

    memcpy(out + 5, buf, len);

    return out;
};