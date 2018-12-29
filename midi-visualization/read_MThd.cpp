#include "midi.h"
#include "endianness.h"

bool read_mthd(std::istream& in, MThd* mthd) {
	bool result;
	result = read(in, mthd);

	if (header_id(mthd->header) != "MThd")
		//|| (mthd->header.size != sizeof(in))
	{
		result = false;
	}
	switch_endianness(&mthd->header.size);
	switch_endianness(&mthd->type);
	switch_endianness(&mthd->ntracks);
	switch_endianness(&mthd->division);

	return result;
}
