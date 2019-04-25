#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <utility>
#include <functional>
#include "SynthStream.h"
#include "../gui/Window.h"
#include "generators.h"
#include "effects.h"
#include "tones.h"

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
		generator{ std::forward<GeneratorParams>(params)... },
		left{ [this](double t) {return generator.getSample(t); } },
		right{ [this](double t) {return generator.getSample(t); } },
		stream{ sampleRate, bufferSize, left, right },
		window{ std::make_shared<Window>(0,0,sf::Color::Black) }
	{
	}

	Generator& getGenerator() { return generator; }
	void play() { stream.play(); }
	void stop() { stream.stop(); }

	std::shared_ptr<Window> getGuiElement() const { return window; }

protected:

	const unsigned sampleRate{ 44100 };
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
		const unsigned wWidth{ 800 }, wHeight{ 400 }, menuHeight{ 30 };
		using pos_t = MenuOption::OptionList::ChildPos_t;
		window->setSize(SynthVec2(wWidth,wHeight));
		window->setHeader(menuHeight, "Instrument1");
		window->setMenuBar(menuHeight);
		auto gui = window->getContentFrame();
		auto menu = window->getMenuFrame();
		gui->setChildAlignment(10);
		gui->setCursor(10, 10);

		keyboard.outputTo(
			generator,
			glider
		);

		gui->addChildAutoPos(pitchBender.getFrame());
		gui->addChildAutoPos(glider.getFrame());

		auto kbAABB = keyboard.getGuiElement()->AABB();
		gui->addChild(keyboard.getGuiElement(), 0, wHeight-kbAABB.height);
		
		menu->addChildAutoPos(MenuOption::createMenu(
			100, 30, 15, {
				"Settings", pos_t::Down, {
				}
			}
		));


		generator.addBeforeCallback(pitchBender);
		generator.addAfterCallback(glider);
	}

private:
	KeyboardOutput keyboard;
	PitchBender<DynamicToneSum> pitchBender;
	Glider glider;

};

#endif //INSTRUMENT_H