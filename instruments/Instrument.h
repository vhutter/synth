#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <utility>
#include <functional>

#include "../gui/Window.h"
#include "generators.h"
#include "tones.h"
#include "effects.h"
#include "SynthStream.h"

template<class Generator>
class Instrument
{
public:

	using Generator_t = Generator;

	template<class ...GeneratorParams>
	Instrument(
		const unsigned sampleRate,
		const unsigned bufferSize,
		GeneratorParams&& ...params
	)
		:sampleRate{ sampleRate },
		bufferSize{ bufferSize },
		generator{ std::forward<GeneratorParams>(params)... },
		left{ [this](double t) {return generator.getSample(t); } },
		right{ [this](double t) {return generator.getSample(t); } },
		stream{ sampleRate, bufferSize, left, right },
		window{ std::make_shared<Window>(0,0,sf::Color::Black) }
	{
	}

	Generator& getGenerator() { return generator; }
	unsigned getSampleRate() const { return sampleRate; }
	unsigned getBufferSize() const { return bufferSize; }
	void play() { stream.play(); }
	void stop() { stream.stop(); }

	std::shared_ptr<Window> getGuiElement() const { return window; }

protected:

	const unsigned sampleRate{ 44100 }, bufferSize{ 64 };
	Generator generator;
	std::function<double(double)> left, right;
	SynthStream stream;
	
	std::shared_ptr<Window> window;
};

class Instrument1 : public Instrument<DynamicToneSum>
{
public:
	//using Instrument::Instrument;
	template<class ...BaseParams>
	Instrument1(BaseParams&& ...params) 
		:Instrument(std::forward<BaseParams>(params)...),
		keyboard{},
		pitchBender{},
		glider{ generator.getTimbreModel(), generator.getNotes(), generator.getMaxTones() }
	{
		init();
	}

private:
	void init();

	KeyboardOutput keyboard;
	PitchBender<DynamicToneSum> pitchBender;
	Glider glider;

};

#endif //INSTRUMENT_H