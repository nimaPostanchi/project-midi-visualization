#include "io.h"

uint32_t read_variable_length_integer(std::istream& in)
{
	uint8_t byte = read_byte(in);
	uint8_t msb = byte >> 7;
	uint8_t msb_remover = 0x7f;
	uint32_t result = byte & msb_remover;

	while (msb == 1) {

		uint8_t next_byte = read_byte(in);

		msb = next_byte >> 7;
		next_byte = next_byte & msb_remover;

		result = (result << 7) | next_byte;
	}
	return result;


}         