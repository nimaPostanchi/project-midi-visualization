#include "settings.h"

#ifdef TEST_BUILD

#include "midi.h"
#include "Catch.h"
#include <sstream>
#include <type_traits>
#include <cstddef>


/*
    This part is quite simple.

    Create a POD-type named NOTE which contains all information about a note:
    * The channel it is on (0-15)
    * The note index (0-127)
    * The (absolute) starting time (0-2^32)
    * The duration (0-2^32)

    You also need to define the operators == and !=.
*/


TEST_CASE("Checking that NOTE is defined correctly")
{
    static_assert(std::is_same<decltype(NOTE::channel), uint8_t>::value, "NOTE is lacking member 'channel' of the appropriate type");
    static_assert(std::is_same<decltype(NOTE::note_index), uint8_t>::value, "NOTE is lacking member 'note_index' of the appropriate type");
    static_assert(std::is_same<decltype(NOTE::start), uint32_t>::value, "NOTE is lacking member 'start' of the appropriate type");
    static_assert(std::is_same<decltype(NOTE::duration), uint32_t>::value, "NOTE is lacking member 'duration' of the appropriate type");
    static_assert(std::is_pod<NOTE>::value, "NOTE is not a POD type");
}

TEST_CASE("Comparing equal NOTE objects")
{
    NOTE a{ 0, 1, 4, 10 };
    NOTE b{ 0, 1, 4, 10 };

    CHECK(a == b);
}

TEST_CASE("Comparing unequal NOTE objects (different channels)")
{
    NOTE a{ 0, 1, 4, 10 };
    NOTE b{ 1, 1, 4, 10 };

    CHECK(!(a == b));
}

TEST_CASE("Comparing unequal NOTE objects (different note indices)")
{
    NOTE a{ 0, 1, 4, 10 };
    NOTE b{ 0, 2, 4, 10 };

    CHECK(!(a == b));
}

TEST_CASE("Comparing unequal NOTE objects (different starts)")
{
    NOTE a{ 0, 1, 4, 10 };
    NOTE b{ 0, 1, 3, 10 };

    CHECK(!(a == b));
}

TEST_CASE("Comparing unequal NOTE objects (different durations)")
{
    NOTE a{ 0, 1, 4, 10 };
    NOTE b{ 0, 1, 4, 20 };

    CHECK(!(a == b));
}


#endif
