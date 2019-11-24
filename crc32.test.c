#include <stdio.h>
#include "./crc32.c"

int crc32_test()
{
    printf("Testing crc functions\n");

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

    printf("\n");
    return 0;

    // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
    // http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
}