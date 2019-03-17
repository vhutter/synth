#ifndef TONES_H_INCLUDED
#define TONES_H_INCLUDED

#include <vector>
#include <atomic>
#include <array>

#include "generators.h"
#include "guiElements.h"


#include <iostream>

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


static TimbreModel CustomToneModel{
	{
		TimbreModel::ToneSkeleton{ 1., 1.,  waves::sine },
		TimbreModel::ToneSkeleton{ 3., 0.3, waves::sine },
	}
};

template<std::size_t maxTones>
class CustomTone1 : public DynamicToneSum<maxTones>
{
	using CompoundTone = CompoundGenerator<Tone>;
	using DynamicTone = DynamicCompoundGenerator<Tone>;
	using Base_t = DynamicToneSum<maxTones>;

public:

	CustomTone1(
		GuiElement& gui, 
		const TimbreModel& model, 
		const std::vector<Note>& notes, 
		typename Base_t::after_t after = {}
	);
	void onKeyEvent(unsigned key, SynthKey::State keyState);

private:

	std::vector<DynamicTone> generateTones(TimbreModel model, const std::vector<Note>& notes);

};

template<std::size_t maxTones>
std::vector<typename CustomTone1<maxTones>::DynamicTone> CustomTone1<maxTones>::generateTones(
	TimbreModel model, 
	const std::vector<Note>& notes
)
{
	std::vector<DynamicTone> tones;
	tones.reserve(notes.size());
	for (auto& note : notes) {
		tones.emplace_back(model(note));
	}
	return tones;
}

template<std::size_t maxTones>
CustomTone1<maxTones>::CustomTone1(
	GuiElement& gui, 
	const TimbreModel& model, 
	const std::vector<Note>& notes,
	typename Base_t::after_t after
)
	:Base_t::DynamicToneSum{
		{
			generateTones(model, notes),
			{},
			after
		}
}
{
	// Pitch slider
	std::shared_ptr sliderPitch{ Slider::DefaultSlider("Pitch", -1, 1, 100, 50, [this](const Slider& this_slider) {
		static double lastValue = 0;
		const double dif = this_slider.getValue() - lastValue;
		std::lock_guard lock(*this);
		lastValue = lastValue + dif;
		this->modifyMainPitch(this->time(), this->getMainFreq() + this_slider.getValue() * 1 / 9 * this->getMainFreq());
	}) };
	sliderPitch->setFixed(true);

	gui.addChildren({
		sliderPitch,
	});
}

template<std::size_t maxTones>
void CustomTone1<maxTones>::onKeyEvent(unsigned keyIdx, SynthKey::State keyState)
{
	if (keyState == SynthKey::State::Pressed) {
		Base_t::components[keyIdx].start(this->time());
	}
	else {
		Base_t::components[keyIdx].stop(this->time());
	}
}

#endif //TONES_H_INCLUDED