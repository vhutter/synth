#ifndef TONES_H_INCLUDED
#define TONES_H_INCLUDED

#include <vector>
#include <atomic>

#include "generators.h"

static TimbreModel Sine13{
	{
		TimbreModel::ToneSkeleton{ 1., 1.,  waves::sine },
		TimbreModel::ToneSkeleton{ 3., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 5., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 7., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 9., 0.3, waves::sine },
	}
};

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