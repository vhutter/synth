#include "synthMain.h"

#include "gui/GuiElements.h"
#include "instruments/tones.h"
#include "instruments/effects.h"
#include "instruments/SynthStream.h"
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

	const unsigned maxNotes{ 5 }, sampleRate{ 44100 };
	auto notes = generateNotes(0, 2);
	auto tones = generateTones<Tone>(Sine13, notes);
	auto generator = DynamicToneSum(tones, maxNotes);

	auto keyboard = KeyboardOutput();
	auto glider = Glider(Sine13, notes, maxNotes);
	auto pitchBender = PitchBender(generator);
	auto echo = EchoEffect(sampleRate, 0.3, 0.6);
	auto volume = VolumeControl();
	auto debugFilter = DebugFilter();

	keyboard.outputTo(
		generator,
		glider
	);

	auto debugWindow = std::make_shared<Window>(debugFilter.getFrame());
	debugWindow->setHeader(30, "Debug");
	debugWindow->setVisibility(false);


	gui->addChild( keyboard.getGuiElement(), 50, 700 );
	gui->addChild( debugWindow, 600, 50);

	gui->setChildAlignment(10);
	gui->setCursor(10, 10);

	gui->addChildAutoPos(volume.getFrame());
	gui->addChildAutoPos(pitchBender.getFrame());
	gui->addChildAutoPos(glider.getFrame() );

	menu->addChildAutoPos(MenuOption::createMenu(
		100, 30, 15, {
			"View", pos_t::Down, {{
				"Debug", debugWindow},
			}
		}
	));
	

	generator.addBeforeCallback(pitchBender);
	generator.addAfterCallback(glider);
	generator.addAfterCallback(echo);
	generator.addAfterCallback(volume);
	generator.addAfterCallback(debugFilter);

	const auto& generateSample = [&](double t) -> double {
		std::lock_guard<DynamicToneSum> lock(generator);
		double sample = generator.getSample(t);
		return sample;
	};

	SynthStream synth(sampleRate, 32, generateSample, generateSample);
	synth.play();

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
