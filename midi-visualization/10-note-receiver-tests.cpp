#include "settings.h"

#ifdef TEST_BUILD

#include "midi.h"
#include "Catch.h"
#include <vector>


/*
    Now is the time to define your own EventReceiver, one that collects the notes from a MTrk chunk.

    There are two note-related events you need to keep an eye on: note on and note off.
    They carry the same extra data: a relative timestamp, a channel, a note index and a velocity.
    You will need to pair up the note on and note off events (same note on the same channel)
    and 

    One slight complication: a note on with velocity 0 is equivalent with a note off.
    In other words, there are two ways to 'turn off' a note. The reason for this
    is that having this allows to make better use of running status, thereby
    potentially saving many bytes.

    Notes are played on a channel. Note events should only be paired if they're on the same channel.
    We could write one class that can keep track of all notes on all channels,
    but generally it's a better idea to spread complexity across multiple classes.
    This is why we'll first create a class that specializes in a single channel (NoteFilter),
    and deal with multiple channels in a separate class.


    Create a subclass of EventReceiver named NoteFilter. A NoteFilter object is
    initialized with a channel and a std::vector<NOTE> in which it will store all
    notes encountered on the given channel. For example,

        std::vector<NOTE> notes;
        NoteFilter filter(0, &notes);
        read_mtrk(in, filter);
        // notes contains all notes on channel 0

    The tests assume the notes are ordered by the position of their note off event in the track;
    this should be the easiest order to achieve. However, the order is not important for the visualization,
    so feel free to modify the tests and make them expect the notes in an order of your liking.
    
*/


TEST_CASE("NoteFilter (channel 0) with single note (channel 0, number 5, from 0, duration 100)")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(0, 0, 5, 255);
    filter.note_off(100, 0, 5, 255);

    REQUIRE(notes.size() == 1);

    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
}

TEST_CASE("NoteFilter (channel 2) with single note (channel 2, number 5, from 0, duration 100)")
{
    std::vector<NOTE> notes;
    NoteFilter filter(2, &notes);

    filter.note_on(0, 2, 5, 255);
    filter.note_off(100, 2, 5, 255);

    REQUIRE(notes.size() == 1);

    CHECK(notes[0] == NOTE{ 2, 5, 0, 100 });
}

TEST_CASE("NoteFilter (channel 2) with single note (channel 0, number 5, from 0, duration 100)")
{
    std::vector<NOTE> notes;
    NoteFilter filter(2, &notes);

    filter.note_on(0, 0, 5, 255);
    filter.note_off(100, 0, 5, 255);

    REQUIRE(notes.size() == 0);
}

TEST_CASE("NoteFilter with two consecutive notes")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(0, 0, 5, 255);
    filter.note_off(100, 0, 5, 255);
    filter.note_on(100, 0, 7, 255);
    filter.note_off(100, 0, 7, 255);

    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{0, 5, 0, 100});
    CHECK(notes[1] == NOTE{ 0, 7, 200, 100 });
}

TEST_CASE("NoteFilter with ABab")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(100, 0, 10, 255);
    filter.note_on(200, 0, 15, 255);
    filter.note_off(300, 0, 10, 255);
    filter.note_off(400, 0, 15, 255);

    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 10, 100, 500 });
    CHECK(notes[1] == NOTE{ 0, 15, 300, 700 });
}

TEST_CASE("NoteFilter with AabB")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(100, 0, 50, 255);
    filter.note_on(200, 0, 40, 255);
    filter.note_off(400, 0, 40, 255);
    filter.note_off(300, 0, 50, 255);

    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 40, 300, 400 });
    CHECK(notes[1] == NOTE{ 0, 50, 100, 900 });
}

TEST_CASE("NoteFilter interprets note on event with velocity 0 as a note off")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(100, 0, 50, 255);
    filter.note_on(200, 0, 50, 0);

    REQUIRE(notes.size() == 1);
    CHECK(notes[0] == NOTE{ 0, 50, 100, 200 });
}

TEST_CASE("NoteFilter interprets note on event with velocity 0 as a note off (2 notes)")
{
    std::vector<NOTE> notes;
    NoteFilter filter(0, &notes);

    filter.note_on(100, 0, 50, 255);
    filter.note_on(200, 0, 40, 255);
    filter.note_off(400, 0, 40, 0);
    filter.note_off(300, 0, 50, 0);

    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 40, 300, 400 });
    CHECK(notes[1] == NOTE{ 0, 50, 100, 900 });
}

TEST_CASE("NoteFilter, 5 simultaneous notes")
{
    std::vector<NOTE> notes;

    NoteFilter filter(0, &notes);

    filter.note_on(0, 0, 0, 255);
    filter.note_on(0, 0, 1, 255);
    filter.note_on(0, 0, 2, 255);
    filter.note_on(0, 0, 3, 255);
    filter.note_on(0, 0, 4, 255);

    filter.note_on(1000, 0, 0, 0);
    filter.note_on(0, 0, 1, 0);
    filter.note_on(0, 0, 2, 0);
    filter.note_on(0, 0, 3, 0);
    filter.note_on(0, 0, 4, 0);

    REQUIRE(notes.size() == 5);
    CHECK(notes[0] == NOTE{ 0, 0, 0, 1000 });
    CHECK(notes[1] == NOTE{ 0, 1, 0, 1000 });
    CHECK(notes[2] == NOTE{ 0, 2, 0, 1000 });
    CHECK(notes[3] == NOTE{ 0, 3, 0, 1000 });
    CHECK(notes[4] == NOTE{ 0, 4, 0, 1000 });
}

#endif
