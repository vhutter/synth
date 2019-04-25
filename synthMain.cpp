#include "synthMain.h"

#include "gui/GuiElements.h"
#include "instruments/tones.h"
#include "instruments/effects.h"
#include "instruments/SynthStream.h"
#include "instruments/Instrument.h"

#include "gui.h"

int synthMain(int argc, char** argv)
{
	const unsigned wWidth{ 1600 }, wHeight{ 1000 }, menuHeight{ 30 };
	sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Basic synth");

	std::shared_ptr mainWindow = std::make_shared<Window>(0, 30, sf::Color::Black);
	mainWindow->setSize({ SynthFloat(wWidth), SynthFloat(wHeight - menuHeight) });
	mainWindow->setMenuBar(menuHeight);

	setupGui(mainWindow, window);


	auto gui = mainWindow->getContentFrame();
	auto menu = mainWindow->getMenuFrame();
	using pos_t = MenuOption::OptionList::ChildPos_t;
	gui->setChildAlignment(10);
	gui->setCursor(10, 10);

	const unsigned maxNotes{ 5 }, sampleRate{ 44100 }, bufferSize{ 16 };
	auto notes = generateNotes(0, 2);

	Instrument1 inst(sampleRate, bufferSize, Sine13, notes, maxNotes);
	gui->addChild(inst.getGuiElement(), 50, 50);



	auto volume = VolumeControl();
	gui->addChildAutoPos(volume.getFrame());

	auto delay = DelayEffect(sampleRate, 0.3, 0.6);
	auto delayWindow = std::make_shared<Window>(delay.getFrame());
	delayWindow->setHeader(30, "Delay");
	delayWindow->setVisibility(false);
	gui->addChildAutoPos(delayWindow);

	auto debugFilter = DebugFilter();
	auto debugWindow = std::make_shared<Window>(debugFilter.getFrame());
	debugWindow->setHeader(30, "Debug");
	debugWindow->setVisibility(false);
	gui->addChildAutoPos(debugWindow);
	


	menu->addChildAutoPos(MenuOption::createMenu(
		100, 30, 15, {
			"View", pos_t::Down, {{
				"Debug", debugWindow}, {
				"Effects", {{
					"Delay", delayWindow},
				}}
			}
		}
	));

	inst.getGenerator().addAfterCallback(delay);
	inst.getGenerator().addAfterCallback(volume);
	inst.getGenerator().addAfterCallback(debugFilter);

	inst.play();

	MidiContext midiContext;

	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);
	while (window.isOpen()) {
		static sf::Event event;
		static MidiEvent midiEvent;

		while (midiContext.pollEvent(midiEvent)) {
			mainWindow->forwardEvent(midiEvent);
		}
		while (window.pollEvent(event)) {
			mainWindow->forwardEvent(event);
		}

		window.clear(sf::Color::Black);
		window.draw(*mainWindow);
		window.display();
	}

	return 0;
}
