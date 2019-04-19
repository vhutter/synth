#include "synthMain.h"

#include "guiElements.h"
#include "tones.h"
#include "effects.h"
#include "gui.h"
#include "synth.h"

int synthMain(int argc, char** argv)
{
	const unsigned sampleRate(44100);
	const unsigned maxNotes{ 5 };
	const unsigned wWidth{ 1600 }, wHeight{ 1000 }, menuHeight{ 30 };
	sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Basic synth");

	std::shared_ptr mainWindow = std::make_shared<Window>(0, 30, sf::Color::Black);
	mainWindow->setSize({ SynthFloat(wWidth), SynthFloat(wHeight - menuHeight) });
	mainWindow->setMenuBar(menuHeight);

	setupGui(*mainWindow, window);


	std::shared_ptr gui = mainWindow->getContentFrame();
	auto keyboard = KeyboardOutput();
	gui->addChildren({ keyboard.getGuiElement() });

	auto notes = generateNotes(0, 2);
	auto tones = generateTones<Tone>(Sine13, notes);
	auto generator = DynamicToneSum(tones, maxNotes);
	auto glideEffect = Glider(*gui, Sine13, notes, maxNotes);
	keyboard.outputTo(
		generator,
		glideEffect
	);

	generator.addBeforeCallback(PitchBender(*gui, generator));

	generator.addAfterCallback(glideEffect);
	generator.addAfterCallback(EchoEffect(*gui, sampleRate, 0.3, 0.6));
	generator.addAfterCallback(VolumeControl(*gui));
	generator.addAfterCallback(DebugFilter(*gui));

	const auto& generateSample = [&](double t) -> double {
		std::lock_guard<DynamicToneSum> lock(generator);
		double sample = generator.getSample(t);
		return sample;
		return 0.;
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
