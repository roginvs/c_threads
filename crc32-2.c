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
       for (uint8_t i = 0; i < 8; ++i)
       {
              uint8_t bit = (byte >> i) & 1;
              if (bit == 1)
              {
                     result = result ^ poly << i;
              };
       };
       return result;
}

void init_table()
{
       for (uint32_t i = 0; i < 0x100; ++i)
       {
              table[i] = _crc32_for_byte((uint8_t)i);
       }
}

/*
gcc -o /tmp/crc crc32-2.c && /tmp/crc 
*/
int main()
{
       if (_crc32_for_byte(0) != 0)
       {
              return 1;
       };
       if (_crc32_for_byte(1) != poly)
       {
              return 2;
       };
       if (_crc32_for_byte(2) != (uint32_t)(poly << 1))
       {
              return 4;
       };
       if (_crc32_for_byte(4) != (uint32_t)(poly << 2))
       {
              return 4;
       };

       if (_crc32_for_byte(5) != (uint32_t)((poly << 2) ^ (poly << 0)))
       {
              return 5;
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
       printf("Done");
}