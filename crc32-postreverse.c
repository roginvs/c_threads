#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

/** This is crc table. Will be filled using fill_crc_table function */
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

void init_crc_table()
{
       for (uint32_t i = 0; i < 0x100; i++)
       {
              table[i] = _crc32_for_byte((uint8_t)i);
       }
}

/** Do a one-byte polynom division */
void poly_reminder_step(uint8_t next_byte, uint32_t *crc)
{
       uint8_t shifted_byte = *((uint8_t *)(crc) + 3);

       uint32_t xoring = table[shifted_byte];

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
void poly_reminder(const uint8_t *data, uint32_t n_bytes, uint32_t *crc)
{
       for (uint32_t i = 0; i < n_bytes; i++)
       {
              uint8_t next_byte = data[i];
              poly_reminder_step(next_byte, crc);
       }
}



/** Reverse bit in while 32-bit variable */
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

void crc32(const uint8_t *data, uint32_t length, uint32_t *crc)
{
       /*
       In summary:

       - Reflect input bytes
       - Add 4 zero bytes to input
       - Xor first 4 input bytes with 0xFF
       - Perform polynom division
       - Xor final crc by 0xFFFFFFFF
       - Reflect all bits (32) in crc

*/
       *crc = 0x0;
       for (uint32_t i = 0; i < 4 && i < length; i++)
       {
              uint8_t next_byte = reverse_bits[data[i] ^ 0xFF];
              poly_reminder_step(next_byte, crc);
       }
       for (uint32_t i = 4; i < length; i++)
       {
              uint8_t next_byte = reverse_bits[data[i]];
              poly_reminder_step(next_byte, crc);
       };
       poly_reminder_step(length <= 3 ? 0xFF : 0x00, crc);
       poly_reminder_step(length <= 2 ? 0xFF : 0x00, crc);
       poly_reminder_step(length <= 1 ? 0xFF : 0x00, crc);
       poly_reminder_step(length <= 0 ? 0xFF : 0x00, crc);
       *crc = *crc ^ 0xFFFFFFFF;
       *crc = reflect_int32(*crc);
}