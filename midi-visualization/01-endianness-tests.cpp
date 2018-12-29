#include "settings.h"

#ifdef TEST_BUILD

#include "endianness.h"
#include "Catch.h"


/*
    The MIDI file format uses a big-endian format for multibyte integers, while your processor expects a little-endian format.
    In short, the difference is the order in which digits are written. The number twelve is traditionally written "12" in decimal,
    i.e. the digits are ordered in decreasing "weight" (in our case, first the tens, followed by the units).
    It could have been the other way around: writing "21" for twelve would make as much sense.

    The same is true for processors: the big-endian processors use the same order we humans do: from left to right, we start with the 'weighty' digits.
    Little-endian processors choose the opposite order.

    An important question is, what do we mean by 'digits' in the case of processors? The most intuitive interpretation would be
    to think of bits, i.e. 4 is written 0b0100 in big endian but 0b0010 in little endian. However, that is NOT how it works.
    Instead, digits correspond to bytes.

    Say we have the 32-bit value 0x12345678 (where 1 is the weighty digit). The bytes are
      0x12 0x34 0x56 0x78

    In big-endian format, this would be written
      0x12 0x34 0x56 0x78
    i.e. just the same (since big-endian and human notation are the same.)

    In little-endian format, it would become
      0x78 0x56 0x34 0x12

    Note how the digits within bytes have not been reordered.


    Write **two** functions switch_endianness(x) that change the endianness of x, once where x is a uint16_t and once where it is a a uint32_t.
    There are multiple ways of doing this:
    * You could reinterpret_cast the given integer as an array of bytes. Reversing an array of 2 or 4 bytes is quite easy (you don't even need to use a loop).
    * You could use bitwise operators (&, |, >>, <<) to extract the bits from the integers.
    * You could start multiplying and dividing to get access to the integer's bytes. Even though this might feel the most familiar to you, I feel this is actually the most complicated approach.
*/


//progma pack for not letting compiler to correct memory


namespace
{
    void test_switch_endianness_16bit(uint16_t x, uint16_t expected)
    {
        switch_endianness(&x);

        CHECK(x == expected);
    }

    void test_switch_endianness_32bit(uint32_t x, uint32_t expected)
    {
        switch_endianness(&x);

        CHECK(x == expected);
    }
}


#define TEST_ENDIANNESS_16(given, expected)   TEST_CASE("Switching endianness of " #given " should yield " #expected) { test_switch_endianness_16bit(given, expected); }
#define TEST_ENDIANNESS_32(given, expected)   TEST_CASE("Switching endianness of " #given " should yield " #expected) { test_switch_endianness_32bit(given, expected); }

TEST_ENDIANNESS_16(0x0000, 0x0000)
TEST_ENDIANNESS_16(0x0001, 0x0100)
TEST_ENDIANNESS_16(0x0100, 0x0001)
TEST_ENDIANNESS_16(0x1000, 0x0010)
TEST_ENDIANNESS_16(0x0054, 0x5400)
TEST_ENDIANNESS_16(0x1234, 0x3412)

TEST_ENDIANNESS_32(0x00000000, 0x00000000)
TEST_ENDIANNESS_32(0x00000001, 0x01000000)
TEST_ENDIANNESS_32(0x00000010, 0x10000000)
TEST_ENDIANNESS_32(0x00000100, 0x00010000)
TEST_ENDIANNESS_32(0x00001000, 0x00100000)
TEST_ENDIANNESS_32(0x00010000, 0x00000100)
TEST_ENDIANNESS_32(0x00100000, 0x00001000)
TEST_ENDIANNESS_32(0x01000000, 0x00000001)
TEST_ENDIANNESS_32(0x10000000, 0x00000010)
TEST_ENDIANNESS_32(0xAABBCCDD, 0xDDCCBBAA)
TEST_ENDIANNESS_32(0x12345678, 0x78563412)

#endif
