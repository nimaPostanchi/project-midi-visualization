#include "midi.h"

bool operator==(NOTE a, NOTE b)
{
	return (a.channel == b.channel) && (a.note_index == b.note_index) && (a.start == b.start) && (a.duration == b.duration);
}

bool operator!= (NOTE a, NOTE b)
{
	return !(a == b);
}