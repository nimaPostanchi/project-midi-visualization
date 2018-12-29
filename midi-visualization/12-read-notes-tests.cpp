#include "settings.h"

#ifdef TEST_BUILD

#include "midi.h"
#include "Catch.h"
#include <vector>
#include <sstream>


/*

    Write a function

        bool read_notes(std::istream&, std::vector<NOTE>*)

    that reads all notes from the given input stream. You should
    assume that the input stream represents a complete MIDI file,
    i.e., you need to read the MThd and all MTrk chunks,
    collect all note-related data and store it in the
    given vector.

    Combine a series of NoteFilters (one per channel) into a single EventMulticaster
    which you pass along to read_mtrk. Repeat this for each MTrk.

*/


TEST_CASE("read_notes, zero tracks")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x00, // Number of tracks
        0x01, 0x00, // Division
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 0);
}

TEST_CASE("read_notes, no notes in track")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 4, // MTrk size
        0x00, char(0xFF), 0x2F, 0x00 // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 0);
}

TEST_CASE("read_notes, single note")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 12, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        100, char(0b1000'0000), 5, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00 // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;
    
    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 1);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
}

TEST_CASE("read_notes, two notes on same track and channel")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 20, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        100, char(0b1000'0000), 5, char(255), // Note off
        100, char(0b1001'0000), 8, char(255), // Note on
        100, char(0b1000'0000), 8, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00 // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
    CHECK(notes[1] == NOTE{ 0, 8, 200, 100 });
}

TEST_CASE("read_notes, two notes on same track different channels")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 20, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        100, char(0b1000'0000), 5, char(255), // Note off
        100, char(0b1001'0010), 8, char(255), // Note on
        100, char(0b1000'0010), 8, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00 // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
    CHECK(notes[1] == NOTE{ 2, 8, 200, 100 });
}

TEST_CASE("read_notes, two notes on different tracks")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x02, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 12, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        100, char(0b1000'0000), 5, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00, // End of track
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 12, // MTrk size
        0, char(0b1001'0000), 88, char(255), // Note on
        100, char(0b1000'0000), 88, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00 // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
    CHECK(notes[1] == NOTE{ 0, 88, 0, 100 });
}

TEST_CASE("read_notes, two notes using running status")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 18, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        0, 6, char(255), // Note on
        100, char(0b1000'0000), 5, char(255), // Note off
        0, 6, char(255), // Note off
        0x00, char(0xFF), 0x2F, 0x00, // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
    CHECK(notes[1] == NOTE{ 0, 6, 0, 100 });
}

TEST_CASE("read_notes, two notes using note on with 0 velocity")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division
        'M', 'T', 'r', 'k',
        0x00, 0x00, 0x00, 17, // MTrk size
        0, char(0b1001'0000), 5, char(255), // Note on
        0, 6, char(255), // Note on
        100, 5, 0, // Note "off"
        0, 6, 0, // Note "off"
        0x00, char(0xFF), 0x2F, 0x00, // End of track
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(read_notes(ss, &notes));
    REQUIRE(notes.size() == 2);
    CHECK(notes[0] == NOTE{ 0, 5, 0, 100 });
    CHECK(notes[1] == NOTE{ 0, 6, 0, 100 });
}

TEST_CASE("read_notes, missing track")
{
    char buffer[] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06, // MThd size
        0x00, 0x01, // Type
        0x00, 0x01, // Number of tracks
        0x01, 0x00, // Division        
    };
    std::string data(buffer, sizeof(buffer));
    std::stringstream ss(data);
    std::vector<NOTE> notes;

    REQUIRE(!read_notes(ss, &notes));
}

#endif
