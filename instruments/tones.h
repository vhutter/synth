#ifndef TONES_H_INCLUDED
#define TONES_H_INCLUDED

#include <vector>
#include <atomic>

#include "generators.h"

const TimbreModel& Sine13();

template<typename WaveGenerator>
std::vector<typename Dynamic<Composite<WaveGenerator>>> generateTones(
	TimbreModel model,
	const std::vector<Note>& notes
)
{
	std::vector<Dynamic<Composite<WaveGenerator>>> tones;
	tones.reserve(notes.size());
	for (auto& note : notes) {
		tones.emplace_back(model(note));
	}
	return tones;
}
#endif //TONES_H_INCLUDED