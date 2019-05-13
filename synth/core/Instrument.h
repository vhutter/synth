#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <utility>
#include <functional>

#include "../gui/Window.h"
#include "generators.h"
#include "tones.h"
#include "effects.h"
#include "SynthStream.h"

class Instrument
{
public:
	Instrument(const std::string& title);

	const std::string& getTitle() const;
	std::shared_ptr<Window> getGuiElement() const;

protected:
	std::string title;
	const unsigned wWidth{ 1000 }, wHeight{ 600 }, menuHeight{ getConfig("defaultHeaderSize") };
	std::shared_ptr<Window> window;
};

class KeyboardInstrument : public Instrument
{
public:
	KeyboardInstrument(
		const std::string& title,
		const TimbreModel& timbreModel,
		const ADSREnvelope& env,
		const std::vector<Note>& notes,
		unsigned maxTones
	);

	DynamicToneSum& getGenerator() { return generator; }

private:
	DynamicToneSum generator;
	KeyboardOutput keyboard;
	PitchBender<DynamicToneSum> pitchBender;
	Glider glider;

};

class InputInstrument : public Instrument
{
public:
	class GeneratorProxy : public SampleGenerator<GeneratorProxy>
	{
	public:
		void feedSample(const double& sample);
		double getSampleImpl(double t) const;

	protected:
		double sample;
	};

	InputInstrument(const std::string& title);

	GeneratorProxy& getGenerator();
	void operator() (const double& sample);

private:
	GeneratorProxy generator;
	std::atomic<bool> isOn{ false };
	std::shared_ptr<Button> button{ Button::OnOffButton(isOn) };
};

#endif //INSTRUMENT_H