#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

uint8_t reverse_bits[] =
    {
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

uint32_t table[0x100];

uint32_t poly = 0x04C11DB7;

uint32_t _crc32_for_byte(uint8_t byte)
{
       uint32_t result = 0;
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
}

void init_table()
{
       for (uint32_t i = 0; i < 0x100; i++)
       {
              table[i] = _crc32_for_byte((uint8_t)i);
       }
}

void poly_reminder(const uint8_t *data, uint32_t n_bytes, uint32_t *crc)
{
       for (uint32_t i = 0; i < n_bytes; i++)
       {
              uint8_t shifted_byte = *((uint8_t *)(crc) + 3);

              uint32_t xoring = table[shifted_byte];

              uint8_t next_byte = data[i];

              /*
              printf("i=%i shifted_byte=%02x xoring=%02x next_byte=%02x curCrc=%08x shiftedCrc=%08x nextCrc=%08x\n",
                     i, shifted_byte, xoring, next_byte,
                     *crc,
                     (*crc << 8) | next_byte,
                     ((*crc << 8) | next_byte) ^ xoring);
*/

              *crc = (*crc << 8) | next_byte;

              *crc = *crc ^ xoring;
       }
}

void _clean(uint8_t *data, uint32_t len)
{
       for (uint32_t i = 0; i < len; ++i)
       {
              data[i] = 0;
       }
}

uint32_t reflect_int32(uint32_t in)
{
       uint32_t r = in;
       *((uint8_t *)(&r) + 0) = reverse_bits[*((uint8_t *)(&r) + 0)];
       *((uint8_t *)(&r) + 1) = reverse_bits[*((uint8_t *)(&r) + 1)];
       *((uint8_t *)(&r) + 2) = reverse_bits[*((uint8_t *)(&r) + 2)];
       *((uint8_t *)(&r) + 3) = reverse_bits[*((uint8_t *)(&r) + 3)];

       uint8_t t;
       t = *((uint8_t *)(&r) + 0);
       *((uint8_t *)(&r) + 0) = *((uint8_t *)(&r) + 3);
       *((uint8_t *)(&r) + 3) = t;

       t = *((uint8_t *)(&r) + 1);
       *((uint8_t *)(&r) + 1) = *((uint8_t *)(&r) + 2);
       *((uint8_t *)(&r) + 2) = t;
       return r;
}
/*
gcc -o /tmp/crc crc32-2.c && /tmp/crc 
*/
int main()
{

       if (_crc32_for_byte(0) != 0x0)
       {
              return 1;
       };
       if (_crc32_for_byte(1) != 0x04C11DB7)
       {
              return 2;
       };
       if (_crc32_for_byte(2) != (uint32_t)(0x09823B6E))
       {
              return 4;
       };
       if (_crc32_for_byte(4) != (uint32_t)(0x130476DC))
       {
              return 4;
       };

       if (_crc32_for_byte(5) != (uint32_t)(0x17C56B6B))
       {
              return 5;
       };

       if (_crc32_for_byte(0x40) != (uint32_t)((0x34867077)))
       {
              printf("err 6 %08x\n", _crc32_for_byte(0x40));
              return 6;
       };

       if (_crc32_for_byte(0x41) != (uint32_t)((0x30476DC0)))
       {
              printf("err 7 %08x\n", _crc32_for_byte(0x40));
              return 7;
       };

       if (_crc32_for_byte(0xFF) != (uint32_t)((0xB1F740B4)))
       {
              printf("err 8 %08x\n", _crc32_for_byte(0x40));
              return 8;
       };

       /*
       for (uint8_t i = 0; i < 10; ++i)
       {
              printf("Byte %02x = %08x\n", i, _crc32_for_byte(i));
       }
       */
       printf("Crc tests are fine\n");
       printf("Creating table\n");
       init_table();
       if (table[1] != poly)
       {
              return 1;
       };
       printf("Table done\n");

       uint32_t crc = 0;
       uint8_t data[8] = {0};
       _clean(data, 8);
       poly_reminder(data, 8, &crc);
       if (crc != 0)
       {
              printf("Err %08x\n", crc);
              return 1;
       };

       // Plain division
       crc = 0x0;
       _clean(data, 8);
       data[0] = 0x6b; // 6b656b0a
       data[1] = 0x65;
       data[2] = 0x6b;
       data[3] = 0x0a;
       poly_reminder(data, 8, &crc);
       if (crc != 0x5F2FC346)
       {
              printf("Crc 1 err %08x\n", crc);
              return 1;
       }

       // Add final xor
       crc = 0x0;
       _clean(data, 8);
       data[0] = 0x6b; // 6b656b0a
       data[1] = 0x65;
       data[2] = 0x6b;
       data[3] = 0x0a;
       poly_reminder(data, 8, &crc);
       crc = crc ^ 0xFFFFFFFF;
       if (crc != 0xA0D03CB9)
       {
              printf("Crc 2 err %08x\n", crc);
              return 1;
       }

       // Add "initial value" xoring
       crc = 0x0;
       _clean(data, 8);
       data[0] = 0x6b; // 6b656b0a
       data[1] = 0x65;
       data[2] = 0x6b;
       data[3] = 0x0a;
       data[0] = data[0] ^ 0xFF;
       data[1] = data[1] ^ 0xFF;
       data[2] = data[2] ^ 0xFF;
       data[3] = data[3] ^ 0xFF;
       poly_reminder(data, 8, &crc);
       crc = crc ^ 0xFFFFFFFF;
       if (crc != 0x67D4E1C2)
       {
              printf("Crc 3 err %08x\n", crc);
              return 1;
       }

       printf("0x6b656b0a crc = %08x\n", crc);

       if (reverse_bits[0] != 0x0)
       {
              return 10;
       };
       if (reverse_bits[0x1] != 0x80)
       {
              return 11;
       };
       if (reverse_bits[0x80] != 0x1)
       {
              return 12;
       };

       if (reverse_bits[0b00010010] != 0b01001000)
       {
              return 13;
       };

       // Adding reflecting input
       crc = 0x0;
       _clean(data, 8);
       data[0] = reverse_bits[0x6b]; // 6b656b0a
       data[1] = reverse_bits[0x65];
       data[2] = reverse_bits[0x6b];
       data[3] = reverse_bits[0x0a];
       data[0] = data[0] ^ 0xFF;
       data[1] = data[1] ^ 0xFF;
       data[2] = data[2] ^ 0xFF;
       data[3] = data[3] ^ 0xFF;
       poly_reminder(data, 8, &crc);
       crc = crc ^ 0xFFFFFFFF;
       if (crc != 0xE5BFC5A7)
       {
              printf("Crc 4 err %08x\n", crc);
              return 1;
       }

       // Adding reflecting output
       crc = 0x0;
       _clean(data, 8);
       data[0] = reverse_bits[0x6b]; // 6b656b0a
       data[1] = reverse_bits[0x65];
       data[2] = reverse_bits[0x6b];
       data[3] = reverse_bits[0x0a];
       data[0] = data[0] ^ 0xFF;
       data[1] = data[1] ^ 0xFF;
       data[2] = data[2] ^ 0xFF;
       data[3] = data[3] ^ 0xFF;
       poly_reminder(data, 8, &crc);
       crc = crc ^ 0xFFFFFFFF;
       crc = reflect_int32(crc);

       if (crc != 0xE5A3FDA7)
       {
              printf("Crc 5 err %08x\n", crc);
              return 1;
       }

       printf("Done\n");

       /*
       In summary:

       - Reflect input bytes
       - Add 4 zero bytes to input
       - Xor first 4 input bytes with 0xFF
       - Perform polynom division
       - Xor final crc by 0xFFFFFFFF
       - Reflect all bits (32) in crc

*/
       // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
       // http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
}