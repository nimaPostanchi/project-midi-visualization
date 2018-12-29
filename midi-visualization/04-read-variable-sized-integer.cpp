#include "settings.h"

#ifdef TEST_BUILD

#include "io.h"
#include "Catch.h"
#include <sstream>


/*
    In order to save space, MIDI files make use of variable length integers (VLI).
    The reasoning is as follows: say you need to encode a large number of integers and
    have limited space.

    * On the one hand you can opt to keep the integers small, i.e., use 8 bit per value.
      This is rather limiting since you can only only reach 255, which might not be enough.
    * On the other hand you can use 32 bit per value, so that the range of possible values (0...2^32)
      is more than sufficient. This requires 4 times as much storage however.

    MIDI's solution is to rely on VLIs: integers that adapt their size
    to the actual value they need to represent.

    So, given a stream of bytes from which you need to read a VLI,
    how do you know how many bytes to read? One way would be to prefix the VLI with its size:

        0x03 0x15 0x44 0x7A 0x64 0x45

    In this case, the 0x03 would indicate that the next three bytes together form an integer,
    so that 0x15447A is the actual value of the VLI.
    However, this is not how MIDI's VLI work, as using a while extra byte for the VLI's length
    is a bit too decadent for its tasted.

    Another way (this is the one that MIDI uses, I promise) to encode VLIs is to sacrificate one
    bit per byte, so that only 7 out of 8 bits are used. The last bit is used as an indicator
    of whether you have reached the end of the VLI or not.

    For example, if you need to encode a small value (< 128), 7 bits suffice:
    
        0b0111'1111              (The ' has no meaning and is just a visual aid. This is actually valid C++ syntax.)

    corresponds to 127. If however you need an extra bit (e.g. 128 requires an eight bit),
    you need to split it in 'groups' of 7 bits.

                  0b1000'0000 ---> 0b000'0001  0b000'0000                 <-- 2 groups of 7 bits
        0b1011'0010'1101'0001 ---> 0b000'0010  0b110'0101  0b101'0001     <-- 3 groups of 7 bits

    Each group of 7 bits is then put in a byte, whose remaining 8th bit is
    set to 1 except for the last byte, i.e., if a byte's most significant bit equals 0,
    that byte is the last one that makes part of the VLI.

              0b1000'0000 ---> 0b1000'0001  0b0000'0000
                                 ^            ^

    0b1011'0010'1101'0001 ---> 0b1000'0010  0b1110'0101  0b0101'0001
                                 ^            ^            ^

    
    You now have to write a function

        uint32_t read_variable_length_integer(std::istream&)
        
    that interprets the next bytes of the given istream as bytes forming a VLI.
    This means you need to keep reading bytes as long as the most significant bit is 1.
    For each byte that you read, you need to extract the 7 least significant bits and
    build one large integer with it.

    You can assume that the result will fit in a uint32_t.
*/

TEST_CASE("Reading variable sized integer from { 0x00 }")
{
    char buffer[] = { 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0);
}

TEST_CASE("Reading variable sized integer from { 0x01 }")
{
    char buffer[] = { 0x01 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 1);
}

TEST_CASE("Reading variable sized integer from { 0x7F }")
{
    char buffer[] = { 0x7F };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0x7F);
}

TEST_CASE("Reading variable sized integer from { 0x81, 0x00 }")
{
    char buffer[] = { char(0x81), 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == (1 << 7));
}

TEST_CASE("Reading variable sized integer from { 0x81, 0x80, 0x00 }")
{
    char buffer[] = { char(0x81), char(0x80), 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == (1 << 14));
}

TEST_CASE("Reading variable sized integer from { 0x81, 0x80, 0x80, 0x00 }")
{
    char buffer[] = { char(0x81), char(0x80), char(0x80), 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == (1 << 21));
}

TEST_CASE("Reading variable sized integer from { 0b10000001, 0b10000001, 0b10000001, 0b00000001 }")
{
    char buffer[] = { char(0b10000001), char(0b10000001), char(0b10000001), char(0b00000001) };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0b1000000100000010000001);
}

TEST_CASE("Reading variable sized integer from { 0b10000111, 0b10010001, 0b11010101, 0b00000000 }")
{
    char buffer[] = { char(0b10000111), char(0b10010001), char(0b11010101), char(0b00000000) };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0b0000111001000110101010000000);
}

TEST_CASE("Reading variable sized integer from { 0b10000111, 0b10010001, 0b11010101, 0b00000000, 0 }")
{
    char buffer[] = { char(0b10000111), char(0b10010001), char(0b11010101), char(0b00000000), 0 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0b0000111001000110101010000000);
}

TEST_CASE("Reading variable sized integer from { 0b00000111, 0b10010001, 0b11010101, 0b00000000, 0 }")
{
    char buffer[] = { char(0b00000111), char(0b10010001), char(0b11010101), char(0b00000000), 0 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    auto actual = read_variable_length_integer(ss);

    CHECK(actual == 0b0000111);
}

#endif
