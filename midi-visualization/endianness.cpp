#include "endianness.h"


void switch_endianness(uint16_t* x)
{

	*x = (*x >> 8) |
		((*x << 8) & 0x00FF0000) |
		((*x >> 8) & 0x0000FF00) |
		(*x << 8);
}

void switch_endianness(uint32_t* x)
{
	*x = (*x >> 24) |
		((*x << 8) & 0x00FF0000) |
		((*x >> 8) & 0x0000FF00) |
		(*x << 24);
}
