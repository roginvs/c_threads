#include <string.h>
#include <stdint.h>

char *store_chunk(char *buf, int32_t buf_len, int32_t *outlen, char is_last)
{
    // printf("\n\nGzip store chunk input buf_len=%i\n", buf_len);
    int32_t total_gzip_blocks = buf_len / 0xffff + (buf_len % 0xffff == 0 ? 0 : 1);
    if (buf_len == 0)
    {
        *outlen = 0;
        return NULL;
    };
    //  printf("Gzip blocks = %i\n", total_gzip_blocks);

    *outlen = total_gzip_blocks * 5 + buf_len;
    char *out = (char *)malloc(*outlen);
    //  printf("outlen = %i\n", *outlen);

    for (int16_t current_gzip_block = 0; current_gzip_block < total_gzip_blocks; current_gzip_block++)
    {

        uint16_t len = current_gzip_block == total_gzip_blocks - 1 ? buf_len - current_gzip_block * 0xFFFF
                                                                   : 0xFFFF;
        // printf("Curent current_gzip_block is %i, len =%i\n", current_gzip_block, len);

        int32_t current_out_offset = current_gzip_block * (0xFFFF + 5);

        int8_t is_last_gzip_block = is_last && current_gzip_block == total_gzip_blocks - 1 ? 1 : 0;
        out[current_out_offset] = is_last_gzip_block ? 0b001 : 0b000; // 00 means no compression

        *(int16_t *)(out + current_out_offset + 1) = len;
        *(int16_t *)(out + current_out_offset + 3) = 0xffff - len;

        memcpy(out + current_out_offset + 5, buf + current_gzip_block * 0xFFFF, len);
    }
    return out;
}

/**
 *  TODO: Check len. Block with no compression are not more than 0xFFFF
 * 
 *  Output should be byte-aligned. Use empty "no compression" block in the end
 *   to achieve alignment
 * 
 * */
char *compress_chunk(char *buf, int32_t buf_len, int32_t *outlen, char is_last)
{
    return store_chunk(buf, buf_len, outlen, is_last);
};
