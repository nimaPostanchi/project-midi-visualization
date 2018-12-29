#ifndef IO_H
#define IO_H
#include<sstream>


template<typename T>
bool read(std::istream& in, T* x) {
	int nr_of_bytes = sizeof(T);
	char* buffer = reinterpret_cast<char*>(x);
	
	in.read(buffer, nr_of_bytes);

	if (in) {
		return true;
	}
	else
	{
		return false;
	}		
}

uint8_t read_byte(std::istream& in);

uint32_t read_variable_length_integer(std::istream&);

#endif

