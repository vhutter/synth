#include "synthMain.h"

#include "gui/GuiElements.h"

#include "gui.h"

int synthMain(int argc, char** argv)
{
	const unsigned wWidth{ 1100 }, wHeight{ 600 }, menuHeight{ getConfig("defaultHeaderSize") };
	sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Basic synth");
	std::shared_ptr mainWindow = std::make_shared<Window>(0, menuHeight, sf::Color::Black);
	mainWindow->setSize({ SynthFloat(wWidth), SynthFloat(wHeight - menuHeight) });
	mainWindow->setMenuBar(menuHeight);

	setupGui(mainWindow, window);

	MidiContext midiContext;
	sf::Event event;
	MidiEvent midiEvent;
	while (window.isOpen()) {

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
