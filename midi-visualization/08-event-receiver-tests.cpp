#include "settings.h"

#ifdef TEST_BUILD

#include "tests-util.h"
#include <sstream>

using namespace testutils;

/*

    The time has arrived to deal with MTrk chunks. This is the most difficult part: read_mtrk, the function you need to
    implement, is relatively large (probably 100+ lines). To alleviate your burden, the tests
    have been written in a 'gradual' manner: you can focus on part A of the function, check if that works,
    then part B, check it, etc. It might be a good idea to comment out the tests and
    uncomment them step by step.

    
    An Mtrk chunk contains "events". The relevant events for use are "note on" and "note off",
    which indicate that an instrument starts or stops playing a certain note, respectively.
    It will be necessary, however, to be able to deal with all MIDI events, even those
    that are of no interest to us.
    
    Each event carries some information with it. For example,
    note on events are accompanied by two extra bytes: the note index and the "velocity".
    What velocity means is not important right now, we'll deal with it later.
    Some events have fixed sizes (e.g. always exactly 2 extra bytes),
    some vary in size. It is therefore important that your code recognizes
    all events, otherwise it will not know how many bytes to skip in order to reach
    the next event in the chunk.


    Each event in the MTrk chunk follows this pattern:

        [time-delta] [event-id] [extra-data]

    The time-delta (a variable length integer) expresses how long after the previous event the current event takes place.
    In other words, it works with relative timestamps instead of absolute ones. The reason
    is probably that relative timestamps generally remain small-valued so that one byte will suffice to encode them.

    After the time-delta you'll need to read the event-id, which will determine what kind of extra-data you can expect.
    
    The general algorithm will look something like

        while not end of track reached
            dt = read relative timestamp
            b = read next byte

            if b == NOTE_ON
              note = read byte
              velocity = read byte
              ...
            else if b == NOTE_OFF
              note = read byte
              velocity = read byte
              ...
            else if b == ...
    

    There are three more things to discuss: running status, end of track chunk and inversion of control.


    END OF TRACK CHUNK
    ------------------
    There are two ways to determine the end of the track:
    * The track chunk start with a chunk header, which contains the number of bytes the chunk is made of.
    * A special event indicates the end of the track.

    We will use the second way. Once you encounter the meta-event with type-code 0x2F (as explained
    in the online sources), you know you reached the last event in the current MTrk chunk.


    RUNNING STATUS
    --------------
    If multiple events of the same type (e.g. multiple note ons) follow each other,
    the event-id can be omitted. For example,

        [dt] [NOTE-ON] [note index] [velocity]
        [dt] [NOTE-ON] [note index] [velocity]
        [dt] [NOTE-ON] [note index] [velocity]

    is generally encoded as

        [dt] [NOTE-ON] [note index] [velocity]
        [dt] [note index] [velocity]
        [dt] [note index] [velocity]

    You will need to find to recognize
    whether a byte following a [dt] is a event id or extra data for
    an event of the same type as the previous one.


    INVERSION OF CONTROL
    --------------------
    Inversion of control is a fancy name for how we will process events.
    Say you write a function read_mtrk that manages to successfully read
    all events. But what does it do with these events?

    One approach would be define an event hierarchy: an Event superclass
    and NoteOnEvent, NoteOffEvent, ... subclasses. Every time
    an event is encountered, an object of the corresponding type is
    created. All Event objects could then be collected in a list which is returned.
    This approach is rather inefficient as it assumes you need all events. 
    All event object also need to fit in memory at once.

    A better approach is to pass along an object (which we'll call EventReceiver) with
    a method for each kind of event. For example, the EventReceiver class must contain

        void note_on(uint32_t dt, uint8_t channel, uint8_t note_index, uint8_t velocity);
        void note_off(uint32_t dt, uint8_t channel, uint8_t note_index, uint8_t velocity);

    So, the read_mtrk function receives such an EventReceiver object and each
    time it encounters a 'note on' event, it reads all data related to it
    and calls note_on() on the EventReceiver object.
    
    Say we are simply interested in the number of notes in a track,
    we would simply implement the following EventReceiver:

        class CountingReceiver : public EventReceiver
        {
        public:
            int note_count;

            CounterReceiver() : note_count(0) { }

            void note_on(uint32_t dt, uint8_t channel, uint8_t note_index, uint8_t velocity) { note_count++; }
            void note_off(uint32_t dt, uint8_t channel, uint8_t note_index, uint8_t velocity) { }
            // all other events methods similarly with empty body
        };

        CountingReceiver r;
        read_mtrk(stream, r);
        std::cout << "Track contains " << r.note_count << " notes" << std::endl;

    
    IMPLEMENTATION STEPS
    --------------------
    1. First, you need to define EventReceiver. This class acts as a superclass
       for event receivers. It only needs to contain one virtual method per
       MIDI event, for a total of 9 methods.

    2. Define the function

            bool read_mtrk(std::istream& in, EventReceiver&)

       Start with a minimal body:
       * It starts with reading the CHUNK_HEADER and checking it is indeed an MTrk (return false immediately if that's not the case).
       * Next, add support for meta events (as described in the online sources), i.e. events identified by 0xFF.
         Add code that recognizes the end of a track (meta event with type == 0x2F).
       * Check that the first few tests work.

    3. Gradually add support for more events. Take a look at the tests for the order in which to proceed.

*/


//TEST_CASE("Reading empty MTrk")
//{    
//    char buffer[] = { 
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 0x04, // Length
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//    
//    auto receiver = Builder().meta(0, 0x2F, nullptr, 0).build();
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single zero-length meta event with dt=0")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        0x00, char(0xFF), 0x01, 0x00, // Some zero-length meta event
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(0, 0x01, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single zero-length meta event with dt=1")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        0x01, char(0xFF), 0x01, 0x00, // Some zero-length meta event
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(1, 0x01, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single zero-length meta event with dt=0b1000'0000")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        char(0b1000'0001), 0b0000'0000, char(0xFF), 0x01, 0x00, // Some zero-length meta event
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(0b1000'0000, 0x01, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single zero-length meta event with dt=0b1000'0110")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        char(0b1000'0001), 0b0000'0110, char(0xFF), 0x01, 0x00, // Some zero-length meta event
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(0b1000'0110, 0x01, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single meta event with data = { 0 }")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        0, char(0xFF), 0x05, 0x01, 0x00,
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    char metadata[] { 0 };
//    auto receiver = Builder()
//        .meta(0, 0x05, metadata, sizeof(metadata))
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with single meta event with data = { 0x12, 0x34 }")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        0, char(0xFF), 0x05, 0x02, 0x12, 0x34,
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    char metadata[]{ 0x12, 0x34 };
//    auto receiver = Builder()
//        .meta(0, 0x05, metadata, sizeof(metadata))
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with two meta events")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 12, // Length
//        1, char(0xFF), 0x01, 0x00, // Meta 1
//        1, char(0xFF), 0x02, 0x00, // Meta 2
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(1, 0x01, nullptr, 0)
//        .meta(1, 0x02, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with three meta events")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 12, // Length
//        1, char(0xFF), 0x01, 0x00, // Meta 1
//        1, char(0xFF), 0x02, 0x00, // Meta 2
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .meta(1, 0x01, nullptr, 0)
//        .meta(1, 0x02, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with sysex event with no data")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 7, // Length
//        0, char(0xF0), 0x00, // Sysex
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .sysex(0, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with sysex event with data = {1, 2, 3}")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        0, char(0xF0), 0x03, 1, 2, 3, // Sysex
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    char sysex_data[]{ 1,2,3 };
//    auto receiver = Builder()
//        .sysex(0, sysex_data, sizeof(sysex_data))
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with sysex event at dt = 0b1111'1111")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        char(0b1000'0001), 0b0111'1111, char(0xF0), 0, // Sysex
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .sysex(0b1111'1111, nullptr, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//
//TEST_CASE("Reading MTrk with note off event (dt 0 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        0, char(0b1000'0000), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 1 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        1, char(0b1000'0000), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(1, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'0000), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 1 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'0001), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 1, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 15 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'1111), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 15, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 15 note 1 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'1111), 1, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 15, 1, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 15 note 32 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'1111), 32, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 15, 32, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note off event (dt 0b1'0000000'0000000 channel 15 note 32 velocity 100)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1000'1111), 32, 100, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0b1'0000000'0000000, 15, 32, 100)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        0, char(0b1001'0000), 0, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 1 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        1, char(0b1001'0000), 0, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(1, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 0 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'0000), 0, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 1 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'0001), 0, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 1, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 15 note 0 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'1111), 0, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 15, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 15 note 1 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'1111), 1, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 15, 1, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 15 note 32 velocity 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'1111), 32, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 15, 32, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with note on event (dt 0b1'0000000'0000000 channel 15 note 32 velocity 100)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 10, // Length
//        char(0b1000'0001), char(0b1000'0000), 0b0000'0000, char(0b1001'1111), 32, 100, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0b1'0000000'0000000, 15, 32, 100)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with polyphonic key pressure event (dt 5 channel 3 note 100 pressure 210)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        5, char(0b1010'0011), 100, char(210), // Polyphonic key pressure
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .polyphonic_key_pressure(5, 3, 100, 210)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with control change event (dt 77 channel 7 controller 3 value 55)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        77, char(0b1011'0111), 3, 55, // Control change
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .control_change(77, 7, 3, 55)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with program change event (dt 127 channel 4 program 2)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 7, // Length
//        127, char(0b1100'0100), 2, // Program change
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .program_change(127, 4, 2)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with channel pressure event (dt 128 channel 3 pressure 99)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 8, // Length
//        char(0b1000'0001), 0b0000'0000, char(0b1101'0011), 99, // Channel pressure
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .channel_pressure(128, 3, 99)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with pitch wheel change event (dt 129 channel 15 value 0)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        char(0b1000'0001), 0b0000'0001, char(0b1110'1111), 0, 0, // Pitch wheel change
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .pitch_wheel_change(129, 15, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with pitch wheel change event (dt 129 channel 15 value 5)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        char(0b1000'0001), 0b0000'0001, char(0b1110'1111), 5, 0, // Pitch wheel change
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .pitch_wheel_change(129, 15, 5)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with pitch wheel change event (dt 129 channel 15 value 0x1234)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 9, // Length
//        char(0b1000'0001), 0b0000'0001, char(0b1110'1111), 0x34, 0x12, // Pitch wheel change
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .pitch_wheel_change(129, 15, 0x1234)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}
//
//TEST_CASE("Reading MTrk with erroneous length (too small)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 11, // Length
//        0, char(0b1001'0000), 0, 0, // Note on
//        10, char(0b1000'0000), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 0, 0)
//        .note_off(10, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(!read_mtrk(ss, *receiver));
//}
//
//TEST_CASE("Reading MTrk with erroneous length (too large)")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 13, // Length
//        0, char(0b1001'0000), 0, 0, // Note on
//        10, char(0b1000'0000), 0, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 0, 0)
//        .note_off(10, 0, 0, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(!read_mtrk(ss, *receiver));
//}
//
//TEST_CASE("Reading MTrk, multiple note on events without running status")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 12, // Length
//        0, char(0b1001'0000), 0, 0, // Note on
//        10, char(0b1001'0000), 5, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 0, 0)
//        .note_on(10, 0, 5, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//}
//
//TEST_CASE("Reading MTrk, multiple note on events with running status")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 11, // Length
//        0, char(0b1001'0000), 0, 0, // Note on
//        10, 5, 0, // Note on
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 0, 0)
//        .note_on(10, 0, 5, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//}
//
//TEST_CASE("Reading MTrk, multiple note off events with running status")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 14, // Length
//        0, char(0b1000'0001), 0, 0, // Note off
//        10, 5, 0, // Note off
//        20, 9, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_off(0, 1, 0, 0)
//        .note_off(10, 1, 5, 0)
//        .note_off(20, 1, 9, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//}
//
//TEST_CASE("Reading MTrk consisting of note on, note on, note off, note off")
//{
//    char buffer[] = {
//        'M', 'T', 'r', 'k',
//        0x00, 0x00, 0x00, 18, // Length
//        0, char(0b1001'0000), 50, char(255), // Note on
//        0, 49, char(255), // Note on
//        10, char(0b1000'0000), 50, 0, // Note off
//        0, 49, 0, // Note off
//        0x00, char(0xFF), 0x2F, 0x00 // End of track
//    };
//    std::string data(buffer, sizeof(buffer));
//    std::stringstream ss(data);
//
//    auto receiver = Builder()
//        .note_on(0, 0, 50, 255)
//        .note_on(0, 0, 49, 255)
//        .note_off(10, 0, 50, 0)
//        .note_off(0, 0, 49, 0)
//        .meta(0, 0x2F, nullptr, 0)
//        .build();
//
//    REQUIRE(read_mtrk(ss, *receiver));
//    receiver->check_finished();
//}

#endif
