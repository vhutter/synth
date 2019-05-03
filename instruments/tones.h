#ifndef TONES_H_INCLUDED
#define TONES_H_INCLUDED

#include <vector>
#include <atomic>

#include "generators.h"

const TimbreModel& Sine();
const TimbreModel& Saw();
const TimbreModel& Square();
const TimbreModel& Triangle();
const TimbreModel& Sines1();
const TimbreModel& Sines2();
const TimbreModel& SinesTriangles();

template<typename WaveGenerator>
std::vector<typename Dynamic<Composite<WaveGenerator>>> generateTones(
	TimbreModel model,
	const std::vector<Note>& notes,
	const ADSREnvelope& env = {}
)
{
	std::vector<Dynamic<Composite<WaveGenerator>>> tones;
	tones.reserve(notes.size());
	for (auto& note : notes) {
		tones.emplace_back(model(note, env));
	}
	return tones;
}
#endif //TONES_H_INCLUDED