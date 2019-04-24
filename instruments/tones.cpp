#include "tones.h"

std::vector<Note> generateNotes(int from, int to)
{
	std::vector<Note> notes;
	for (int i = from; i <= to; ++i)
	for (auto note : baseNotes) notes.push_back(note * pow(2, i));
	notes.push_back(baseNotes[0] * pow(2, to + 1));
	return notes;
}