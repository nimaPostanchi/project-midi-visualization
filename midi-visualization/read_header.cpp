#include "midi.h"
#include "endianness.h"


bool read_header(std::istream& in, CHUNK_HEADER* header) {
	//char buffer[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 0 };
	//std::string data(read_variable_length_integer(in), sizeof(in));
	//std::stringstream ss(data);

	//uint32_t b = read_variable_length_integer(ss);
	//header = reinterpret_cast<CHUNK_HEADER*>(buffer);
	
	bool result = read(in, header);
	switch_endianness(&header->size);
	return result;
	//return read(in, c);
}