#include <stdint.h>

/** This is crc table. Will be filled using fill_crc_table function */
uint32_t table[0x100];

// 0x04C11DB7 but all bits are reversed
uint32_t poly = 0xEDB88320;

uint32_t _crc32_for_byte(uint8_t byte)
{
    uint32_t result = 0;

    uint8_t poly_high_byte = *((uint8_t *)&poly);

    printf("Table for byte 0x%02x\n", byte);
    for (uint8_t i = 0; i < 8; ++i)
    {

        uint8_t bit = (byte >> i) & 1;

        printf("Bit %i value = %i\n", i, bit);
        if (bit == 1)
        {

            result = result ^ (poly >> (7 - i));
            // byte = byte ^ (1 >> bit_pos); // This can be skipped
            byte = byte ^ (poly_high_byte << (i + 1));
        };
    };

    return result;
    /*
    uint8_t poly_high_byte = *((uint8_t *)&poly + 3);

    // printf("Table value for byte %i\n", byte);
    for (uint8_t i = 0; i < 8; ++i)
    {
        uint8_t bit_pos = 7 - i;

        uint8_t bit = (byte >> bit_pos) & 1;

        if (bit == 1)
        {
            // printf("Bit %i is set\n", i);
            result = result ^ (poly << bit_pos);
            // byte = byte ^ (1 << bit_pos); // This can be skipped
            byte = byte ^ (poly_high_byte >> (i + 1));
        };
    };
    return result;
    */
}
