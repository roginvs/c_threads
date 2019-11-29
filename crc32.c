#include <stdint.h>

/** This is crc table. Will be filled using fill_crc_table function */
uint32_t table[0x100];

// 0x04C11DB7 but all bits are reversed
uint32_t poly = 0xEDB88320;

uint32_t _crc32_for_byte(uint8_t byte)
{
       uint32_t result = 0;

       uint8_t poly_high_byte = *((uint8_t *)&poly);

       //  [ ] [ ] [ ] [ ]

       // printf("Table for byte 0x%02x\n", byte);

       // x^7 is on the zero position
       // So, starting from zero to 7 means
       // going through x^7 to x^0
       for (uint8_t i = 0; i < 8; ++i)
       {

              uint8_t bit = (byte >> i) & 1;

              // printf("Bit %i value = %i\n", i, bit);
              if (bit == 1)
              {
                     // Move native poly to the left to the 7-i positions
                     // Which is move right
                     result = result ^ (poly >> (7 - i));
                     // byte = byte ^ (1 >> bit_pos); // This can be skipped
                     byte = byte ^ (poly_high_byte << (i + 1));
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
       uint8_t shifted_byte = *((uint8_t *)crc);

       uint32_t xoring = table[shifted_byte];

       /*
              printf("i=%i shifted_byte=%02x xoring=%02x next_byte=%02x curCrc=%08x shiftedCrc=%08x nextCrc=%08x\n",
                     i, shifted_byte, xoring, next_byte,
                     *crc,
                     (*crc << 8) | next_byte,
                     ((*crc << 8) | next_byte) ^ xoring);
*/

       // Shifting crc with next byte
       *crc = (*crc >> 8) | (next_byte << 24);

       // And then xor with table value from shifted byte
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

uint32_t poly_multiple(uint32_t a, uint32_t b)
{
       // Multiplication result
       // It is sliding from x^62..x^31 to x^31..0 range
       // Why x^62 ? Because we have two polynoms which
       //  highest degree is x^31
       // Total amounts of window shifts is 31
       uint32_t window = 0;

       // printf("Multiplication 0x%08x * 0x%08x\n", a, b);

       // Go through highest degree to lowest
       for (uint8_t i = 0; i < 32; i++)
       {
              uint8_t bit_value = (b >> i) & 1;

              // printf("Bit as pos %i value=%i\n", i, bit_value);
              if (bit_value == 1)
              {
                     // Bit is set. This means that we should add
                     //  (a * x^(31-i)) % poly
                     // into result

                     // Our window is shifted left now for 31-i items
                     // This means it is already multiplied by x^(31-i)
                     // So, we just adding "a" into window

                     // printf("  Adding 'a' to window. Before=0x%08x after=0x%08x\n", window, window ^ a);
                     window = window ^ a;
              }

              // If it is not the last cycle, then we need to shift window
              if (i != 31)
              {
                     // Now prepare for shift
                     uint8_t window_highest_degree_bit = window & 1;
                     // printf("  window=0x%08x shiftedWindow=0x%08x shiftedBit=%i\n", window, window >> 1, window_highest_degree_bit);
                     window = window >> 1;
                     if (window_highest_degree_bit == 1)
                     {
                            window = window ^ poly;
                            // printf("  windowAfterPolyXor=0x%08x\n", window);
                     }
              }
       }

       return window;
}

uint32_t power_of_n(uint32_t power)
{
       if (power == 0)
       {
              return 0b1 << (31 - 0);
              ;
       }
       else if (power == 1)
       {
              return 0b1 << (31 - 8);
       }
       else if (power == 2)
       {
              return 0b1 << (31 - 8 - 8);
       }
       else if (power == 3)
       {
              return 0b1 << (31 - 8 - 8 - 8);
       };
       uint32_t half_power = power / 2;
       uint32_t poly_of_half_power = power_of_n(half_power);
       uint32_t poly_of_floor_even = poly_multiple(poly_of_half_power, poly_of_half_power);
       if (power % 2 == 0)
       {
              return poly_of_floor_even;
       }
       else
       {
              return poly_multiple(poly_of_floor_even, power_of_n(1));
       }
}

uint32_t crc32(const uint8_t *data, uint32_t length)
{
       /*
       In summary:

       - (skip due to changed crc table) Reflect input bytes
       - Add 4 zero bytes to input
       - Xor first 4 input bytes with 0xFF
       - Perform polynom division
       - Xor final crc by 0xFFFFFFFF
       - (skip due to changed crc accumulator) Reflect all bits (32) in crc 

*/
       uint32_t crc = 0x0;
       for (uint32_t i = 0; i < 4 && i < length; i++)
       {
              uint8_t next_byte = data[i] ^ 0xFF;
              poly_reminder_step(next_byte, &crc);
       }
       for (uint32_t i = 4; i < length; i++)
       {
              uint8_t next_byte = data[i];
              poly_reminder_step(next_byte, &crc);
       };
       poly_reminder_step(length <= 3 ? 0xFF : 0x00, &crc);
       poly_reminder_step(length <= 2 ? 0xFF : 0x00, &crc);
       poly_reminder_step(length <= 1 ? 0xFF : 0x00, &crc);
       poly_reminder_step(length <= 0 ? 0xFF : 0x00, &crc);
       crc = crc ^ 0xFFFFFFFF;
       return crc;
}

uint32_t crc32_partial_block(const uint8_t *data, uint32_t block_length, uint32_t bytes_before, uint32_t bytes_after)
{
       uint8_t xor_border = bytes_before > 4 ? 4 : bytes_before;

       uint32_t crc = 0x0;
       for (uint32_t i = 0; i < xor_border && i < block_length; i++)
       {
              uint8_t next_byte = data[i] ^ 0xFF;
              poly_reminder_step(next_byte, &crc);
       }
       for (uint32_t i = xor_border; i < block_length; i++)
       {
              uint8_t next_byte = data[i];
              poly_reminder_step(next_byte, &crc);
       };

       uint32_t crc_shift = power_of_n(bytes_after + 4);

       crc = poly_multiple(crc, crc_shift);

       if (bytes_after == 0)
       {
              uint32_t total_length = bytes_before + block_length;
              poly_reminder_step(total_length <= 3 ? 0xFF : 0x00, &crc);
              poly_reminder_step(total_length <= 2 ? 0xFF : 0x00, &crc);
              poly_reminder_step(total_length <= 1 ? 0xFF : 0x00, &crc);
              poly_reminder_step(total_length <= 0 ? 0xFF : 0x00, &crc);
       };

       crc = crc ^ 0xFFFFFFFF;
       return crc;
}

uint32_t crc32_block_combine(uint32_t crc1, uint32_t crc2)
{
       return crc1 ^ crc2;
}