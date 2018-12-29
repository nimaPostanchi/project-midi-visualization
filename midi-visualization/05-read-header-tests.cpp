#include "settings.h"

#ifdef TEST_BUILD

#include "midi.h"
#include "Catch.h"
#include <sstream>
#include <type_traits>
#include <cstddef>


/*
    A MIDI file consists of a number of chunks. The first one is a MThd chunk,
    which describes the contents of the MIDI file. It is followed by one or more
    MTrk chunks, representing "tracks".

    While it may seem intuitive, we will in our implementation not
    see tracks as instruments. Later on, you will see that notes
    are played on a "channel", and that is what we'll interpret
    as instruments and will assign separate colors to.


    Each chunk starts with a header. This header contains two pieces of information:
    * What type of chunk it is (either MThd or MTrk)
    * How much data it contains in bytes

    We'll start off with defining a "POD-type", i.e., "Plain Old Data" (http://en.cppreference.com/w/cpp/concept/PODType)
    The exact definition is a bit complicated, but let's just say it is
    a struct/class with only public fields (this is more limiting than the actual
    definition of a POD type, but it is both simpler and sufficient for our purposes.)
    
    For example,

        struct Color
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

    is an example of a POD.

    We will use it as a "mold": we read in raw bytes in memory, then place
    over them this POD type to tell C++ how the bytes should be grouped into
    larger values (e.g. these 4 bytes form a uint32_t) and what names to give them.
    This is done using reinterpret_cast. For example,

        uint8_t bytes[] = { 255, 0, 0 };
        Color* c = reinterpret_cast<Color*>(bytes);
        // c.r == 255, c.g == 0, c.b == 0
        bytes[1] = 128;
        // c.r == 255, c.g == 128, c.b == 0
        c.b = 192;
        // bytes == { 255, 128, 192 }

    In other words, you look at the same piece of memory in two different ways.
    Note that this is very efficient: the cast itself is actually a no-operation.
    It exists only in the compiler's head.


    These pages describe the MIDI format:

        http://valentin.dasdeck.com/midi/midifile.htm
        http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html

    Look for information about the chunk header (NOT the header chunk, that's the MThd chunk).
    Define a POD-type named CHUNK_HEADER that contains the necessary fields of the right type in the right order.
    The first test below will check whether you defined it correctly.


    Next,

        bool read_header(std::istream&, CHUNK_HEADER*)

    which reads the appropriate number of bytes from the given istream
    and stores them to wherever the given pointer points.
    Do not hardcode the number of bytes. Your code must
    say "read as many bytes as CHUNK_HEADER is large",
    not "read 15 bytes" (15 is not the correct number btw).
    Don't forget you can simply rely on functions which
    you wrote earlier.

    read_header should return true if all went well, false it not (e.g. insufficient bytes in the stream).
*/


TEST_CASE("Checking that CHUNK_HEADER is defined correctly")
{
    static_assert(std::is_same<decltype(CHUNK_HEADER::id), char[4]>::value, "CHUNK_HEADER is lacking member 'id' of the appropriate type");
    static_assert(std::is_same<decltype(CHUNK_HEADER::size), uint32_t>::value, "CHUNK_HEADER is lacking member 'size' of appropriate type");
    static_assert(sizeof(CHUNK_HEADER) == sizeof(char[4]) + sizeof(uint32_t), "CHUNK_HEADER does not have the right size");
    static_assert(std::is_pod<CHUNK_HEADER>::value, "CHUNK_HEADER is not a POD type");

    static_assert(offsetof(CHUNK_HEADER, id) == 0, "CHUNK_HEADER's id field does not have the correct offset");
    static_assert(offsetof(CHUNK_HEADER, size) == 4, "CHUNK_HEADER's size field does not have the correct offset");
}

TEST_CASE("Reading CHUNK_HEADER { 'M', 'T', 'h', 'd', 0, 0, 0, 0 }")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 0 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    CHUNK_HEADER header;
    read_header(ss, &header);
    std::string type(header.id, sizeof(header.id));

    CHECK(type == "MThd");
    CHECK(header.size == 0);
}

TEST_CASE("Reading CHUNK_HEADER { 'M', 'T', 'h', 'd', 0x01, 0, 0, 0 }")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0x01, 0, 0, 0 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    CHUNK_HEADER header;

    REQUIRE(read_header(ss, &header));

    SECTION("Checking id")
    {
        std::string type(header.id, sizeof(header.id));
        CHECK(type == "MThd");
    }

    SECTION("Checking size")
    {
        INFO("Have you taken into account endianness?")
            CHECK(header.size == 0x01000000);
    }
}

TEST_CASE("Reading CHUNK_HEADER { 'M', 'T', 'r', 'k', 0x01, 0x02, 0x03, 0x04 }")
{
    char buffer[] = { 'M', 'T', 'r', 'k', 0x01, 0x02, 0x03, 0x04 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    CHUNK_HEADER header;

    REQUIRE(read_header(ss, &header));

    SECTION("Checking id")
    {
        std::string type(header.id, sizeof(header.id));
        CHECK(type == "MTrk");
    }

    SECTION("Checking size")
    {
        INFO("Have you taken into account endianness?");
        CHECK(header.size == 0x01020304);
    }
}

TEST_CASE("Reading CHUNK_HEADER { 'M', 'T', 'r', 'k', 0x45, 0x12, 0x75, 0x66 }")
{
    char buffer[] = { 'M', 'T', 'r', 'k', 0x45, 0x12, 0x75, 0x66 };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    CHUNK_HEADER header;

    REQUIRE(read_header(ss, &header));

    SECTION("Checking id")
    {
        std::string type(header.id, sizeof(header.id));
        CHECK(type == "MTrk");
    }

    SECTION("Checking size")
    {
        INFO("Have you taken into account endianness?");
        CHECK(header.size == 0x45127566);
    }
}

#endif
