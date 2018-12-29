#include "settings.h"

#ifdef TEST_BUILD

#include "io.h"
#include "Catch.h"
#include <sstream>

/*
    Write a function read_byte that takes an istream& and reads a single byte (type: uint8_t) from it.
    Rely on your previously written read function.

    Usage

        uint8_t b = read_byte(in);
        // b now contains the next byte in the stream


    Implementation

    You can assume the stream does indeed have a byte left in it, i.e., no need to check if read() fails.
    (This is technically not very robust, but I didn't want to burden you with exceptions or other error handling stuff)
*/


TEST_CASE("Reading single byte from { 0x00 }")
{
    char buffer[] = { 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);

    CHECK(read_byte(ss) == buffer[0]);
}

TEST_CASE("Reading single byte from { 0x01 }")
{
    char buffer[] = { 0x01 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);

    CHECK(read_byte(ss) == buffer[0]);
}

TEST_CASE("Reading single byte from { 0x12 }")
{
    char buffer[] = { 0x12 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);

    CHECK(read_byte(ss) == buffer[0]);
}

TEST_CASE("Reading single byte from { 0x12, 0x34 }")
{
    char buffer[] = { 0x12, 0x34 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);

    CHECK(read_byte(ss) == buffer[0]);
}

TEST_CASE("Reading single byte from { 0x12, 0x34, 0x56, 0x78 }")
{
    char buffer[] = { 0x12, 0x34, 0x56, 0x78 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);

    CHECK(read_byte(ss) == buffer[0]);
}

#endif
