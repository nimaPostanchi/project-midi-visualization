#include "settings.h"

#ifdef TEST_BUILD

#include "io.h"
#include "Catch.h"
#include <sstream>

/*

    Write a function 
    
        bool read<T>(std::istream& in, T* x)

    that reads an arbitrary T from the given input stream.
    

    Example usage:

        uint32_t x;
        read(std::cin, &x); // reads 4 bytes from std::cin and stores the result in x

        struct Color {
          uint8_t r;
          uint8_t g;
          uint8_t b;
        };
        Color c;
        read(std::cin, &c); // reads 3 bytes and stores them as r, g, b values


    Implementation:

    std::istream itself has a method "read" which you'll need. Give it a char* p and a number N, and
    it will read N bytes for the stream and write the bytes to the position pointed to by p.

    Your read function needs to perform the following steps:
    * Find out the size of T in bytes (e.g. if T = uint32_t, the size is 4)
    * Cast parameter x to char* so as to be able to pass it to std::istream's read-method
    * Use in.read(...) to read in the bytes
    * If all goes well, return true. However, it is possible the input stream does not contain a
      sufficient amount of bytes (e.g. you want to read a data structure 10 bytes long
      but there are only 5 bytes left in the stream). In this case the function should return false.
      The simplest way to detect this lack of bytes is to just call istream's read method.
      Internally, the istream object keeps a fail bit which is set to true by its read method
      if it fails. Look online to find out how to query this bit.

*/


namespace
{
#   pragma pack(1)
    struct Foo
    {
        uint8_t a;
        uint16_t b;
        uint32_t c;
    };
}


TEST_CASE("Reading uint32_t from { 0x00, 0x00, 0x00, 0x00 }")
{
    char buffer[] = { 0x00, 0x00, 0x00, 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    uint32_t result;

    CHECK(read(ss, &result));
    CHECK(result == 0);
}

TEST_CASE("Reading uint32_t from { 0x01, 0x00, 0x00, 0x00 }")
{
    char buffer[] = { 0x01, 0x00, 0x00, 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    uint32_t result;

    CHECK(read(ss, &result));
    CHECK(result == 1);
}

TEST_CASE("Reading uint32_t from { 0x00, 0x01, 0x00, 0x00 }")
{
    char buffer[] = { 0x00, 0x01, 0x00, 0x00 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    uint32_t result;

    CHECK(read(ss, &result));
    CHECK(result == 0x0100);
}

TEST_CASE("Reading uint32_t from { 0x12, 0x34, 0x56, 0x78 }")
{
    char buffer[] = { 0x12, 0x34, 0x56, 0x78 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    uint32_t result;

    CHECK(read(ss, &result));
    CHECK(result == 0x78563412);
}

TEST_CASE("Reading Foo from { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }")
{
    char buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    Foo result;

    CHECK(read(ss, &result));
    CHECK(result.a == 0x01);
    CHECK(result.b == 0x0302);
    CHECK(result.c == 0x07060504);
}

TEST_CASE("Reading uint32_t from { 0x01, 0x02, 0x03 } should fail")
{
    char buffer[] = { 0x01, 0x02, 0x03 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    Foo result;

    CHECK(!read(ss, &result));
}

TEST_CASE("Reading Foo from { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } should fail")
{
    char buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    Foo result;

    CHECK(!read(ss, &result));
}

#endif
