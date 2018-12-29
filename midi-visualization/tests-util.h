#ifndef TESTS_UTIL_H
#define TESTS_UTIL_H

#include "Catch.h"
#include "midi.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <list>


namespace testutils
{
    struct Event
    {
        uint32_t dt;

        virtual ~Event() { }
    };

    struct MetaEvent : public Event
    {
        uint8_t type;
        std::vector<char> data;
    };

    struct SysexEvent : public Event
    {
        std::vector<char> data;
    };

    struct MidiEvent : public Event
    {
        uint8_t channel;
    };

    struct NoteOnEvent : public MidiEvent
    {
        uint8_t note;
        uint8_t velocity;
    };

    struct NoteOffEvent : public MidiEvent
    {
        uint8_t note;
        uint8_t velocity;
    };

    struct PolyphonicEvent : public MidiEvent
    {
        uint8_t note;
        uint8_t pressure;
    };

    struct ControlChangeEvent : public MidiEvent
    {
        uint8_t controller;
        uint8_t value;
    };

    struct ProgramChangeEvent : public MidiEvent
    {
        uint8_t program;
    };

    struct ChannelPressureEvent : public MidiEvent
    {
        uint8_t pressure;
    };

    struct PitchWheelChangeEvent : public MidiEvent
    {
        uint16_t value;
    };

    class TestEventReceiver : public EventReceiver
    {
    private:
        std::list<std::unique_ptr<Event>> expected_events;

    public:
        TestEventReceiver(std::list<std::unique_ptr<Event>> expected_events) : expected_events(std::move(expected_events)) { }

        void note_on(uint32_t dt, uint8_t channel, uint8_t note, uint8_t velocity) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<NoteOnEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->note == note);
            CHECK(actual->velocity == velocity);
        }

        void note_off(uint32_t dt, uint8_t channel, uint8_t note, uint8_t velocity) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<NoteOffEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->note == note);
            CHECK(actual->velocity == velocity);
        }

        void polyphonic_key_pressure(uint32_t dt, uint8_t channel, uint8_t note, uint8_t pressure) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<PolyphonicEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->note == note);
            CHECK(actual->pressure == pressure);
        }

        void control_change(uint32_t dt, uint8_t channel, uint8_t controller, uint8_t value) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<ControlChangeEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->controller == controller);
            CHECK(actual->value == value);
        }

        void program_change(uint32_t dt, uint8_t channel, uint8_t program) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<ProgramChangeEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->program == program);
        }

        void channel_pressure(uint32_t dt, uint8_t channel, uint8_t pressure) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<ChannelPressureEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->channel == channel);
            CHECK(actual->pressure == pressure);
        }

        void pitch_wheel_change(uint32_t dt, uint8_t channel, uint16_t value) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<PitchWheelChangeEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->value == value);
        }

        void meta(uint32_t dt, uint8_t type, const char* data, int data_size) override
        {
            {
                INFO("This failure means that your function finds nonexistent events");

                REQUIRE(expected_events.size() != 0);
            }

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<MetaEvent*>(expected.get());
            
            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            CHECK(actual->type == type);
            REQUIRE(actual->data.size() == data_size);

            for (int i = 0; i != data_size; ++i)
            {
                CHECK(actual->data[i] == data[i]);
            }
        }

        void sysex(uint32_t dt, const char* data, int data_size) override
        {
            REQUIRE(expected_events.size() != 0);

            std::unique_ptr<Event> expected = std::move(expected_events.front());
            expected_events.pop_front();
            auto actual = dynamic_cast<SysexEvent*>(expected.get());

            REQUIRE(actual != nullptr);
            CHECK(actual->dt == dt);
            REQUIRE(actual->data.size() == data_size);

            for (int i = 0; i != data_size; ++i)
            {
                CHECK(actual->data[i] == data[i]);
            }
        }

        void check_finished()
        {
            SECTION("Checking that all events have been handled")
            {
                INFO("This failure means that your code did not find all events in the track");

                CHECK(expected_events.size() == 0);
            }
        }
    };

    class Builder
    {
    private:
        std::list<std::unique_ptr<Event>> expected_events;

    public:
        Builder& note_on(uint32_t dt, uint8_t channel, uint8_t note, uint8_t velocity)
        {
            auto event = std::make_unique<NoteOnEvent>();
            event->dt = dt;
            event->channel = channel;
            event->note = note;
            event->velocity = velocity;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& note_off(uint32_t dt, uint8_t channel, uint8_t note, uint8_t velocity)
        {
            auto event = std::make_unique<NoteOffEvent>();
            event->dt = dt;
            event->channel = channel;
            event->note = note;
            event->velocity = velocity;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& polyphonic_key_pressure(uint32_t dt, uint8_t channel, uint8_t note, uint8_t pressure)
        {
            auto event = std::make_unique<PolyphonicEvent>();
            event->dt = dt;
            event->channel = channel;
            event->note = note;
            event->pressure = pressure;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& control_change(uint32_t dt, uint8_t channel, uint8_t controller, uint8_t value)
        {
            auto event = std::make_unique<ControlChangeEvent>();
            event->dt = dt;
            event->channel = channel;
            event->controller = controller;
            event->value = value;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& program_change(uint32_t dt, uint8_t channel, uint8_t program)
        {
            auto event = std::make_unique<ProgramChangeEvent>();
            event->dt = dt;
            event->channel = channel;
            event->program = program;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& channel_pressure(uint32_t dt, uint8_t channel, uint8_t pressure)
        {
            auto event = std::make_unique<ChannelPressureEvent>();
            event->dt = dt;
            event->channel = channel;
            event->pressure = pressure;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& pitch_wheel_change(uint32_t dt, uint8_t channel, uint16_t value)
        {
            auto event = std::make_unique<PitchWheelChangeEvent>();
            event->dt = dt;
            event->channel = channel;
            event->value = value;

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& meta(uint32_t dt, uint8_t type, const char* data, int data_size)
        {
            auto event = std::make_unique<MetaEvent>();
            event->dt = dt;
            event->type = type;
            event->data = std::vector<char>(data, data + data_size);

            expected_events.push_back(std::move(event));

            return *this;
        }

        Builder& sysex(uint32_t dt, const char* data, int data_size)
        {
            auto event = std::make_unique<SysexEvent>();
            event->dt = dt;
            event->data = std::vector<char>(data, data + data_size);

            expected_events.push_back(std::move(event));

            return *this;
        }

        std::unique_ptr<TestEventReceiver> build()
        {
            return std::make_unique<TestEventReceiver>(std::move(expected_events));
        }
    };
}


#endif