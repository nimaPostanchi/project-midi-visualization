#include "midi.h"
#include <iostream>

std::string header_id(const CHUNK_HEADER& c) {
	std::string id(c.id, 4);
	return id;
}


