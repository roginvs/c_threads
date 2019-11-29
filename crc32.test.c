#include <stdio.h>
#include "./crc32.c"

/** Table of bytes with reversed bits */
uint8_t _reverse_bits[] =
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

/** Reverse bit in while 32-bit variable */
uint32_t _reflect_int32(uint32_t in)
{
    uint32_t r = in;
    *((uint8_t *)(&r) + 0) = _reverse_bits[*((uint8_t *)(&r) + 0)];
    *((uint8_t *)(&r) + 1) = _reverse_bits[*((uint8_t *)(&r) + 1)];
    *((uint8_t *)(&r) + 2) = _reverse_bits[*((uint8_t *)(&r) + 2)];
    *((uint8_t *)(&r) + 3) = _reverse_bits[*((uint8_t *)(&r) + 3)];

    uint8_t t;
    t = *((uint8_t *)(&r) + 0);
    *((uint8_t *)(&r) + 0) = *((uint8_t *)(&r) + 3);
    *((uint8_t *)(&r) + 3) = t;

    t = *((uint8_t *)(&r) + 1);
    *((uint8_t *)(&r) + 1) = *((uint8_t *)(&r) + 2);
    *((uint8_t *)(&r) + 2) = t;
    return r;
}

/** Just another memset */
void _clean(uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        data[i] = 0;
    }
}

#define assertEqual(a, b, msg)                                 \
                                                               \
    if ((a) != (b))                                            \
    {                                                          \
        printf("\n");                                          \
        printf("Asserted=0x%08x Obtained=0x%08x\n", (b), (a)); \
        printf("Error with: ");                                \
        printf(msg);                                           \
        printf("\n");                                          \
        return 100;                                            \
    }
;
int crc32_test()
{
    printf("Checking poly\n");
    if (_reflect_int32(poly) != 0x04C11DB7)
    {
        printf("Wrong poly\n");
        return 20;
    };

    printf("Testing reverse bits table\n");
    if (_reverse_bits[0] != 0x0)
    {
        return 10;
    };
    if (_reverse_bits[0x1] != 0x80)
    {
        return 11;
    };
    if (_reverse_bits[0x80] != 0x1)
    {
        return 12;
    };

    if (_reverse_bits[0b00010010] != 0b01001000)
    {
        return 13;
    };

    printf("Testing crc table builder\n");

    if (_crc32_for_byte(0) != 0x0)
    {
        printf("err 1\n");
        return 1;
    };
    if (_crc32_for_byte(_reverse_bits[1]) != _reflect_int32(0x04C11DB7))
    {
        printf("err 2\n");
        return 2;
    };
    if (_crc32_for_byte(_reverse_bits[2]) != _reflect_int32(0x09823B6E))
    {
        printf("err 3\n");
        return 3;
    };
    if (_crc32_for_byte(_reverse_bits[4]) != _reflect_int32(0x130476DC))
    {
        printf("err 4\n");
        return 4;
    };

    if (_crc32_for_byte(_reverse_bits[5]) != _reflect_int32(0x17C56B6B))
    {
        printf("err 5\n");
        return 5;
    };

    if (_crc32_for_byte(_reverse_bits[0x40]) != _reflect_int32((0x34867077)))
    {
        printf("err 6 %08x\n", _crc32_for_byte(0x40));
        return 6;
    };

    if (_crc32_for_byte(_reverse_bits[0x41]) != _reflect_int32((0x30476DC0)))
    {
        printf("err 7 %08x\n", _crc32_for_byte(0x40));
        return 7;
    };

    if (_crc32_for_byte(_reverse_bits[0xFF]) != _reflect_int32((0xB1F740B4)))
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
    init_crc_table();
    if (table[_reverse_bits[0x1]] != poly)
    {
        printf("Table fails\n");
        return 1;
    };
    printf("Table done\n");

    uint32_t crc = 0;
    uint8_t data[8] = {0};
    _clean(data, 8);
    crc = poly_reminder(data, 8);
    if (crc != 0)
    {
        printf("Err %08x\n", crc);
        return 1;
    };

    // Plain division
    crc = 0x0;
    _clean(data, 8);
    data[0] = _reverse_bits[0x6b]; // 6b656b0a
    data[1] = _reverse_bits[0x65];
    data[2] = _reverse_bits[0x6b];
    data[3] = _reverse_bits[0x0a];
    crc = poly_reminder(data, 8);
    if (crc != _reflect_int32(0x5F2FC346))
    {
        printf("Crc 1 err %08x\n", crc);
        return 1;
    }

    /*
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

    // Adding reflecting input
    crc = 0x0;
    _clean(data, 8);
    data[0] = _reverse_bits[0x6b]; // 6b656b0a
    data[1] = _reverse_bits[0x65];
    data[2] = _reverse_bits[0x6b];
    data[3] = _reverse_bits[0x0a];
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
    data[0] = _reverse_bits[0x6b]; // 6b656b0a
    data[1] = _reverse_bits[0x65];
    data[2] = _reverse_bits[0x6b];
    data[3] = _reverse_bits[0x0a];
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

    printf("Poly reminder is fine\n");

    data[0] = 0x6b; // 6b656b0a
    data[1] = 0x65;
    data[2] = 0x6b;
    data[3] = 0x0a;
    crc32(data, 4, &crc);
    if (crc != 0xE5A3FDA7)
    {
        printf("Crc 6 err %08x\n", crc);
        return 1;
    }
    */

    char data2[] = "Vasilii";
    crc = crc32((uint8_t *)data2, 7);
    // crc32 <(echo -n 'Vasilii')
    if (crc != 0x2701c6cc)
    {
        printf("Crc 7 err %08x\n", crc);
        return 1;
    }

    crc = crc32((uint8_t *)data2, 1);
    // crc32 <(echo -n 'Vasilii')
    if (crc != 0x500a1b4c)
    {
        printf("Crc 8 err %08x\n", crc);
        return 1;
    }

    crc = crc32(NULL, 0);
    if (crc != 0x0)
    {
        printf("Crc 9 err %08x\n", crc);
        return 1;
    }

    printf("crc32 s fine\n");

    printf("Tesing poly multiplication\n");
    if (poly_multiple(_reflect_int32(0b0), _reflect_int32(0b0)) != _reflect_int32(0b0))
    {
        printf("Poly multiple err 50\n");
        return 50;
    };

    assertEqual(poly_multiple(_reflect_int32(0b1), _reflect_int32(0b1)), _reflect_int32(0b1), "1*1");

    assertEqual(poly_multiple(_reflect_int32(0b10), _reflect_int32(0b10)), _reflect_int32(0b100), "x*x");

    // Numeric 0b11*0b11 = 1001
    // Poly = x^2 + 1
    assertEqual(poly_multiple(_reflect_int32(0b11), _reflect_int32(0b11)), _reflect_int32(0b101), "(x+1)*(x+1)");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 24), _reflect_int32(0b1)), 0x1, "Multiply by 1");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 24), _reflect_int32(0b10)), poly, "x^31 * x");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 24), _reflect_int32(0b100)), poly >> 1, "x^31 * x^2");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 24), _reflect_int32(0b1000000)), poly >> 5, "x^31 * x^5");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 24), _reflect_int32(0b10000000)), (poly >> 6) ^ poly, "x^31 * x^6");

    assertEqual(poly_multiple(_reflect_int32(0b10000000 << 22), _reflect_int32(0b10000000)), (poly >> 4), "x^29 * x^6");

    assertEqual(poly_multiple(
                    _reflect_int32(0b10000000 << 24 ^ 0b10000000 << 22),
                    _reflect_int32(0b1)),
                _reflect_int32((0b10000000 << 24 ^ 0b10000000 << 22)),
                "(x^31 + x^29) * (1)");

    assertEqual(poly_multiple(
                    _reflect_int32(0b10000000 << 24 ^ 0b10000000 << 22),
                    _reflect_int32(0b10000001)),
                (poly >> 6) ^ poly ^ poly >> 4 ^ _reflect_int32(0b10000000 << 24 ^ 0b10000000 << 22),
                "(x^31 + x^29) * (x^6 + 1)");

    assertEqual(poly_multiple(
                    _reflect_int32(0b10000000000000000),
                    _reflect_int32(0b10000000000000000)),
                poly,
                "x^16 * x^16");

    assertEqual(poly_multiple(
                    _reflect_int32(0b1000000000000000000000000000000),
                    _reflect_int32(0b1000000000000000000000000000000)),
                _reflect_int32(0b11110111000101000010110110100011),
                "x^30 * x^30");

    assertEqual(poly_multiple(
                    _reflect_int32(0b10000000000000000000000000000000),
                    _reflect_int32(0b10000000000000000000000000000000)),
                _reflect_int32(0b11010001000100111001000001010101),
                "x^31 * x^31");

    /*
        a = 0xDEADBEEF = 11011110101011011011111011101111;
        b = 0xBADDCAFE = 10111010110111011100101011111110;
        
        
        http://www.ee.unb.ca/cgi-bin/tervo/calc.pl?num=11011110101011011011111011101111&den=10111010110111011100101011111110&f=m&e=1&m=1

        Product =
        111111001110001100111000010110011010000100000010110100001001010

        crcPoly = 100000100110000010001110110110111

        Remainder = 0b10000100100110001011011010111000
    */

    assertEqual(
        poly_multiple(_reflect_int32(0xDEADBEEF),
                      _reflect_int32(0xBADDCAFE)),
        _reflect_int32(0b10000100100110001011011010111000),
        "Scary polynoms");

    printf("Testing power\n");

    assertEqual(_reflect_int32(power_of_n(0)), 0b1, "x^0");

    assertEqual(_reflect_int32(power_of_n(1)), 0b100000000, "x^(1*8)");

    assertEqual(_reflect_int32(power_of_n(2)), 0b10000000000000000, "x^(2*8)");

    assertEqual(_reflect_int32(power_of_n(3)), 0b1000000000000000000000000, "x^(3*8)");

    assertEqual(_reflect_int32(power_of_n(4)), 0b100110000010001110110110111, "x^(4*8)");

    assertEqual(_reflect_int32(power_of_n(5)), 0b11010010000110011100000111011100, "x^(5*8)");

    assertEqual(_reflect_int32(power_of_n(6)), 0b1110110001010110010000111, "x^(6*8)");

    assertEqual(_reflect_int32(power_of_n(7)), 0b11011100011011011001101010110111, "x^(7*8)");

    printf("Testing poly combine\n");

    /*
        a = 0xDEADBEEF = 11011110101011011011111011101111;
        b = 0xBADDCAFE = 10111010110111011100101011111110;
        c = 0xFEEDFACE = 11111110111011011111101011001110;
        
        abc = 110111101010110110111110111011111011101011011101110010101111111011111110111011011111101011001110
        
        http://www.ee.unb.ca/cgi-bin/tervo/calc.pl?

        crcPoly = 100000100110000010001110110110111
    */

    uint8_t abc[] = {
        _reverse_bits[0b11011110], // 1
        _reverse_bits[0b10101101],
        _reverse_bits[0b10111110],
        _reverse_bits[0b11101111],
        _reverse_bits[0b10111010], // 5
        _reverse_bits[0b11011101],
        _reverse_bits[0b11001010],
        _reverse_bits[0b11111110],
        _reverse_bits[0b11111110], // 9
        _reverse_bits[0b11101101],
        _reverse_bits[0b11111010],
        _reverse_bits[0b11001110],
    };

    assertEqual(
        poly_reminder(abc, 4),
        _reflect_int32(0b11011110101011011011111011101111), "Poly division 4");

    assertEqual(
        poly_reminder(abc, 5),
        _reflect_int32(0b10000000101010110000010001011001), "Poly division 5");

    assertEqual(
        poly_reminder(abc, 6),
        _reflect_int32(0b11000010000010001011100100110011), "Poly division 6");

    assertEqual(
        poly_reminder(abc, 8),
        _reflect_int32(0b11111100000000110000110110011101), "Poly division 8");

    assertEqual(
        poly_reminder(abc, 10),
        _reflect_int32(0b111100100010101010001100101110), "Poly division 10");

    assertEqual(
        poly_multiple(poly_reminder(abc, 5), power_of_n(5)) ^
            poly_reminder(abc + 5, 5),
        poly_reminder(abc, 10), "Poly parts 5+5");

    assertEqual(
        poly_multiple(poly_reminder(abc, 4), power_of_n(8)) ^
            poly_multiple(poly_reminder(abc + 4, 4), power_of_n(4)) ^
            poly_reminder(abc + 8, 4),
        poly_reminder(abc, 12), "Poly parts 4+4+4");

    assertEqual(
        poly_multiple(poly_reminder(abc, 4), power_of_n(6)) ^
            poly_multiple(poly_reminder(abc + 4, 4), power_of_n(2)) ^
            poly_reminder(abc + 8, 2),
        poly_reminder(abc, 10), "Poly parts 4+4+2");

    printf("Testing crc combining\n");
    data[0] = 0x6b ^ 0xFF; // 6b656b0a
    data[1] = 0x65 ^ 0xFF;
    data[2] = 0x6b ^ 0xFF;
    data[3] = 0x0a ^ 0xFF;
    // crc == e5a3fda7
    assertEqual(
        0xFFFFFFFF ^ poly_multiple(poly_reminder(data, 4), power_of_n(4)), 0xe5a3fda7, "Crc combining 1");

    uint8_t data4[] = "My name is Vasilii!";
    uint8_t data4_xored[4] = {data4[0] ^ 0xFF, data4[1] ^ 0xFF, data4[2] ^ 0xFF, data4[3] ^ 0xFF};

    assertEqual(
        0xFFFFFFFF ^
            poly_multiple(
                poly_reminder(data4_xored, 4),
                power_of_n(4)),

        crc32(data4, 4),
        "Crc combining 2");

    assertEqual(
        0xFFFFFFFF ^

            poly_multiple(
                poly_reminder(data4_xored, 4),
                power_of_n(24 - 4)) ^

            poly_multiple(
                poly_reminder(data4 + 4, 16),
                power_of_n(24 - 4 - 16)),

        crc32(data4, 20), "Crc combining 4");

    /*
    printf("Testing crc by parts\n");

    uint8_t data3[] = "This is a test line ";
    crc = crc32(data3, 20);

    uint32_t crc2 = crc32_finallize(crc32_block_combine(
        crc32_partial_block(data3, 10, 0, 10),
        crc32_partial_block(data3 + 10, 10, 10, 0)));
    
    assertEqual(crc, crc2, "Equal blocks");
    */

    return 0;

    // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
    // http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
}