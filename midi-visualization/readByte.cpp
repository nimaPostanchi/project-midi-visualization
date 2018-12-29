#include "io.h"

uint8_t read_byte(std::istream& in) {
	uint8_t x;
	read(in, &x);
	return x;
}
