#include <string.h>
#include <stdint.h>

//
// #include "./crc32.c"

uint8_t *store_chunk(uint8_t *buf, uint32_t buf_len, uint32_t *outlen, uint8_t is_last)
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
    uint8_t *out = (uint8_t *)malloc(*outlen);
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
 *
 *  Output should be byte-aligned. Use empty "no compression" block in the end
 *   to achieve alignment
 * 
 * */
uint8_t *compress_chunk(uint8_t *buf, uint32_t buf_len, uint32_t *outlen, uint32_t *crc, uint32_t bytes_before, uint32_t bytes_after)
{
    uint8_t is_last = bytes_after == 0;
    //
    // Here is supposed to be the compressing logic
    //
    uint8_t *compressed = store_chunk(buf, buf_len, outlen, is_last);
    *crc = crc32_partial_block(buf, buf_len, bytes_before, bytes_after);
    // printf("CRC in compress_chunk len=%i, before=%i, after=%i\n", buf_len, bytes_before, bytes_after);
    return compressed;
};
