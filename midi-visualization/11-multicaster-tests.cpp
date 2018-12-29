#include "settings.h"

#ifdef TEST_BUILD

#include "tests-util.h"
#include <vector>
#include <functional>

using namespace testutils;


/*
    The NoteFilter class you wrote focuses on a single channel.
    In order to get all notes on all sixteen channels, we can't just
    repeatedly call read read_mtrk, as it "consumes" the stream:
    once you got through it, you don't want to rewind and go through
    it 15 more times. Our goal is to have to pass through the
    track chunk only once.

    For this, we introduce the EventMulticaster. The idea is simple:
    it is an EventReceiver that, every that it receives an event,
    send the information to a list of other EventReceivers.

                                      +---> EventReceiver 1
                                      |
        event ---> EventMulticaster --+---> EventReceiver 2
                                      |
                                      +---> EventReceiver 3

    This way, we can construct 16 NoteFilters, one for each channel,
    and give them to an EventMulticaster:

        std::vector<NOTE> notes;
        std::vector<std::shared_ptr<EventReceiver>> note_filters = create 16 NoteFilters that each store their notes in the vector notes.
        EventMulticaster multicaster(note_filters);
        read_mtrk(in, multicaster); // All 16 note filters receive the events and put the notes they find in the vector


    Write a class EventMulticaster that takes a vector of EventReceivers and broadcasts the events it receives
    to all receivers in the vector.
*/

TEST_CASE("Multicaster test, one receiver, one event (note on)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().note_on(1, 2, 3, 4).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.note_on(1, 2, 3, 4);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (note off)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().note_off(1, 2, 3, 4).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.note_off(1, 2, 3, 4);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (polyphonic key pressure)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().polyphonic_key_pressure(10, 5, 20, 30).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.polyphonic_key_pressure(10, 5, 20, 30);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (control change)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().control_change(5, 6, 7, 8).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.control_change(5, 6, 7, 8);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (program change)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().program_change(9, 6, 3).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.program_change(9, 6, 3);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (channel pressure)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().channel_pressure(8, 5, 2).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.channel_pressure(8, 5, 2);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (pitch wheel change)")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().pitch_wheel_change(1, 5, 9).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.pitch_wheel_change(1, 5, 9);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, one event (meta)")
{
    char data[]{ 1, 2, 3, 4, 5 };
    auto create_receiver = [&data]() {
        return std::shared_ptr<TestEventReceiver>(Builder().meta(1, 9, data, sizeof(data)).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.meta(1, 9, data, sizeof(data));

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, one receiver, two events")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().note_on(0, 0, 0, 255).note_off(10, 0, 0, 255).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.note_on(0, 0, 0, 255);
    multicaster.note_off(10, 0, 0, 255);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, two receivers, one event")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().note_on(0, 0, 0, 255).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver(), create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.note_on(0, 0, 0, 255);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

TEST_CASE("Multicaster test, five receivers, one event")
{
    auto create_receiver = []() {
        return std::shared_ptr<TestEventReceiver>(Builder().note_on(1000, 1, 0, 255).build().release());
    };

    std::vector<std::shared_ptr<TestEventReceiver>> receivers{ create_receiver(), create_receiver(), create_receiver(), create_receiver(), create_receiver() };
    EventMulticaster multicaster(std::vector<std::shared_ptr<EventReceiver>>(receivers.begin(), receivers.end()));

    multicaster.note_on(1000, 1, 0, 255);

    for (auto& receiver : receivers)
    {
        receiver->check_finished();
    }
}

#endif
