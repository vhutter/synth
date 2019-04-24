#ifndef TONES_H_INCLUDED
#define TONES_H_INCLUDED

#include <vector>
#include <atomic>
#include <array>

#include "generators.h"

static const std::array<Note, 12> baseNotes = {
	Note::C,
	Note::Cis,
	Note::D,
	Note::Dis,
	Note::E,
	Note::F,
	Note::Fis,
	Note::G,
	Note::Gis,
	Note::A,
	Note::Ais,
	Note::B
};

std::vector<Note> generateNotes(int fromOctave, int toOctave);

static TimbreModel Sine13{
	{
		TimbreModel::ToneSkeleton{ 1., 1.,  waves::sine },
		TimbreModel::ToneSkeleton{ 3., 0.3, waves::sine },
	}
};

template<typename Tone>
std::vector<typename DynamicCompoundGenerator<Tone>> generateTones(
	TimbreModel model,
	const std::vector<Note>& notes
)
{
	std::vector<DynamicCompoundGenerator<Tone>> tones;
	tones.reserve(notes.size());
	for (auto& note : notes) {
		tones.emplace_back(model(note));
	}
	return tones;
}
#endif //TONES_H_INCLUDED