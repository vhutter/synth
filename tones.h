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

	CustomTone1(GuiElement& gui, typename Base_t::after_t after = {});

private:

	// Configuration
	ContinuousFunction glidePitch{ 100 };
	std::atomic<bool> glide{ false };
	std::atomic<double> octave{ 1. };
	std::atomic<double> glideSpeed{ .3 };


	// Gui
	std::shared_ptr<SynthKeyboard> keyboard;

	std::vector<Note> generateNotes(int from, int to);

	std::vector<DynamicTone> generateTones();
};

template<std::size_t maxTones>
std::vector<Note> CustomTone1<maxTones>::generateNotes(int from, int to)
{
	std::vector<Note> notes;
	for (int i = from; i <= to; ++i)
		for (auto note : baseNotes) notes.push_back(note * pow(2, i));
	notes.push_back(baseNotes[0] * pow(2, to + 1));
	return notes;
}

template<std::size_t maxTones>
std::vector<typename CustomTone1<maxTones>::DynamicTone> CustomTone1<maxTones>::generateTones()
{
	const std::vector<Note>& notes{ generateNotes(0,2) };
	TimbreModel myToneModel{ CustomToneModel };
	myToneModel.beforeTone = [this](double t, Tone& tone) {
		tone.note = tone.getMainFreq() * octave;
	};
	// Tone for glide effect
	TimbreModel glidingToneModel{ myToneModel };
	glidingToneModel.beforeTone = [this](double t, Tone& tone) {
		//tone.phase += sin(t*10) * 15 / tone.note;
		tone.note = tone.note * octave;
	};
	glidingToneModel.before = [this](double t, CompoundGenerator<Tone>& input) {
		input.modifyMainPitch(t, glidePitch.getValue(t));
	};
	static CompoundTone glidingTone{ glidingToneModel(notes.front()) };
	myToneModel.after = [this](double t, double& sample) {
		if (glide) {
			glidingTone.modifyMainPitch(t, glidePitch.getValue(t));
			sample = glidingTone.getSample(t);
		}
	};
	//Generation of the final tones
	std::vector<DynamicTone> tones;
	tones.reserve(notes.size());
	for (auto& note : notes) {
		tones.emplace_back(myToneModel(note));
	}
	return tones;
}

template<std::size_t maxTones>
CustomTone1<maxTones>::CustomTone1(GuiElement& gui, typename Base_t::after_t after)
	:Base_t::DynamicToneSum{
		{
			generateTones(),
			{},
			after
		}
}
{
	// GUI

	// Keyboard interaction
	keyboard = std::make_unique<SynthKeyboard>(50, 700, [this](unsigned keyIdx) {
		static int lastKeyIdx{ -1 };
		const auto& kb = keyboard;
		if (kb->isLastEventKeypress()) {
			if (glide) {
				glidePitch.setValueLinear(Base_t::components[keyIdx].getMainFreq(), this->time(), glideSpeed);
				if (lastKeyIdx != -1) Base_t::components[lastKeyIdx].stop(this->time());
			}
			Base_t::components[keyIdx].start(this->time());
			(*kb)[keyIdx].setPressed(true);
			lastKeyIdx = keyIdx;
		}
		else {
			Base_t::components[keyIdx].stop(this->time());
			(*kb)[keyIdx].setPressed(false);
		}
	});

	// Pitch slider
	std::shared_ptr sliderPitch{ Slider::DefaultSlider("Pitch", -1, 1, 100, 50, [this](const Slider& this_slider) {
		static double lastValue = 0;
		const double dif = this_slider.getValue() - lastValue;
		std::lock_guard lock(*this);
		lastValue = lastValue + dif;
		this->modifyMainPitch(this->time(), this->getMainFreq() + this_slider.getValue() * 1 / 9 * this->getMainFreq());
	}) };
	sliderPitch->setFixed(true);

	// Gui for the glide effect
	std::shared_ptr glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, 160, 50, glideSpeed) };
	std::shared_ptr glideButton{ Button::DefaultButton("Glide", 180, 180, glide) };

	// Octave with mouse
	static std::shared_ptr mouseEvents{ std::make_shared<EmptyGuiElement>([this](const SynthEvent& eventArg) {

		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			switch (event.type) {
				case sf::Event::MouseWheelScrolled: {
					std::lock_guard lock(*this);
					if (event.mouseWheelScroll.delta > 0)
						octave = octave * 2.;
					else
						octave = octave * 0.5;
					break;
				}
				default: {

				}
			}
		}
	}) };


	gui.addChildren({
		keyboard,
		sliderPitch,
		glideSpeedSlider,
		glideButton,
		mouseEvents,
		});
}

#endif //TONES_H_INCLUDED