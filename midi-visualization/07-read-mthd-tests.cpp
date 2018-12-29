#include "settings.h"

#ifdef TEST_BUILD

#include "midi.h"
#include "Catch.h"
#include <sstream>
#include <type_traits>
#include <cstddef>


/*
    There are two kinds of chunks: MThd and MTrk. While MTrk chunks can have varying sizes, MThd chunks always
    have the same size. This means we can use the same approach as when reading CHUNK_HEADER: we define
    a POD containing all the pieces of data contained within an MThd and read it in one go.

    Look up what a MThd chunk looks like and define a corresponding POD-type named MThd.
    Next, write

        bool read_mthd(std::istream&, MThd*)

    that reads the data from the given istream and writes it to memory pointed to by the given pointer.
*/


TEST_CASE("Checking that MThd is defined correctly")
{
    static_assert(std::is_same<decltype(MThd::header), CHUNK_HEADER>::value, "MThd is lacking member 'header' of the appropriate type");
    static_assert(std::is_same<decltype(MThd::type), uint16_t>::value, "MThd is lacking member 'type' of the appropriate type");
    static_assert(std::is_same<decltype(MThd::ntracks), uint16_t>::value, "MThd is lacking member 'ntracks' of the appropriate type");
    static_assert(std::is_same<decltype(MThd::division), uint16_t>::value, "MThd is lacking member 'division' of the appropriate type");

	// If this line does not compile, look online for "pragma pack push pop"
    static_assert(sizeof(MThd) == sizeof(CHUNK_HEADER) + 3 * sizeof(uint16_t), "MThd does not have the right size");

    static_assert(offsetof(MThd, header) == 0, "MThd's header field does not have the correct offset");
    static_assert(offsetof(MThd, type) == 8, "MThd's type field does not have the correct offset");
    static_assert(offsetof(MThd, ntracks) == 10, "MThd's ntracks field does not have the correct offset");
    static_assert(offsetof(MThd, division) == 12, "MThd's division field does not have the correct offset");
}

TEST_CASE("Reading MThd from {'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01}")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01 };
    static_assert(sizeof(buffer) == sizeof(MThd), "Bug in tests");
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    MThd mthd;

    REQUIRE(read_mthd(ss, &mthd));
    CHECK(header_id(mthd.header) == "MThd");
    CHECK(mthd.header.size == 6);
    CHECK(mthd.type == 1);
    CHECK(mthd.ntracks == 1);
    CHECK(mthd.division == 1);
}

TEST_CASE("Reading MThd from {'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01}")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01 };
    static_assert(sizeof(buffer) == sizeof(MThd), "Bug in tests");
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    MThd mthd;

    REQUIRE(read_mthd(ss, &mthd));
    CHECK(header_id(mthd.header) == "MThd");
    CHECK(mthd.header.size == 6);
    CHECK(mthd.type == 0);
    CHECK(mthd.ntracks == 5);
    CHECK(mthd.division == 0x0201);
}

TEST_CASE("Reading MThd from {'M', 'T', 'h', 'x', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01} should fail because of wrong id")
{
    char buffer[] = { 'M', 'T', 'h', 'x', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01 };
    static_assert(sizeof(buffer) == sizeof(MThd), "Bug in tests");
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    MThd mthd;

    REQUIRE(!read_mthd(ss, &mthd));
}

//TEST_CASE("Reading MThd from {'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01} should fail because of wrong length")
//{
//    char buffer[] = { 'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x02, 0x01 };
//    static_assert(sizeof(buffer) == sizeof(MThd), "Bug in tests");
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//    MThd mthd;
//
//    REQUIRE(!read_mthd(ss, &mthd));
//}

TEST_CASE("Reading MThd from {'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02} should fail because of too few bytes")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x02 };
    static_assert(sizeof(buffer) < sizeof(MThd), "Bug in tests");
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    MThd mthd;

    REQUIRE(!read_mthd(ss, &mthd));
}

TEST_CASE("Reading MThd from {'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x02} should fail because of too few bytes")
{
    char buffer[] = { 'M', 'T', 'h', 'd', 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x02 };
    static_assert(sizeof(buffer) < sizeof(MThd), "Bug in tests");
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    MThd mthd;

    REQUIRE(!read_mthd(ss, &mthd));
}

#endif
