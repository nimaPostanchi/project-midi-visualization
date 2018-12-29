#ifndef MIDI_H
#define MIDI_H
#include "io.h"
#include "endianness.h"
#include<sstream>
#include <type_traits>

struct CHUNK_HEADER
{
	char id[4];
	uint32_t size;
};

#pragma pack (push, 2)
struct MThd 
{
	CHUNK_HEADER header;
	uint16_t type;
	uint16_t ntracks;
	uint16_t division;
};
#pragma pack(pop)

struct NOTE
{
	uint8_t channel;
	uint8_t note_index;
	uint32_t start;
	uint32_t duration;
};

bool read_header(std::istream& in, CHUNK_HEADER* c);

std::string header_id(const CHUNK_HEADER& c);

bool read_mthd(std::istream& in, MThd* m);

bool operator ==(NOTE, NOTE);

bool operator !=(NOTE, NOTE);

#endif
