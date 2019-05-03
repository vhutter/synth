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
		const std::string title,
		GeneratorParams&& ...params
	)
		:title(title),
		generator{ std::forward<GeneratorParams>(params)... },
		window{ std::make_shared<Window>(wWidth, wHeight) }
	{
		window->setHeader(30, title);
	}

	Generator& getGenerator() { return generator; }
	const std::string& getTitle() const { return title; }
	std::shared_ptr<Window> getGuiElement() const { return window; }

protected:

	Generator generator;
	std::string title;
	const unsigned wWidth{ 1000 }, wHeight{ 600 }, menuHeight{ 30 };
	std::shared_ptr<Window> window;
};

class Instrument1 : public Instrument<DynamicToneSum>
{
public:
	template<class ...BaseParams>
	Instrument1(BaseParams&& ...params) 
		:Instrument(std::forward<BaseParams>(params)...),
		keyboard{ generator.getNotesCount() },
		pitchBender{ generator },
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