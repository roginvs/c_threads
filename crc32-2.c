#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

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

       /*
       printf("\nKek\n");
       crc = 0;
       _clean(data, 8);
       // 6b
       // ???
       // 0x39 - works
       // 0x40 - not works
       //    (0x40 = 0b01000000) = 64
       //   poly = 1 0000 0100 1100 0001 0001 1101 1011 0111
       data[0] = 0x40;

       poly_reminder(data, 5, &crc);

       printf("Lol %08x\n", crc);
*/
       printf("Done\n");

       // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
       // http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
}