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
	EmptyGuiElement gui;
	auto keyboard = KeyboardOutput();
	gui.addChildren({ keyboard.getGuiElement() });

	auto notes = generateNotes(0, 2);
	auto tones = generateTones<Tone>(Sine13, notes);
	auto generator = DynamicToneSum(tones, maxNotes);
	auto glideEffect = Glider(gui, Sine13, notes, maxNotes);
	keyboard.outputTo(
		generator,
		glideEffect
	);

	generator.addBeforeCallback(PitchBender(gui, generator));

	generator.addAfterCallback(glideEffect);
	generator.addAfterCallback(EchoEffect(gui, sampleRate, 0.3, 0.6));
	generator.addAfterCallback(VolumeControl(gui));
	generator.addAfterCallback(DebugFilter(gui));

	const auto& generateSample = [&](double t) -> double {
		std::lock_guard<DynamicToneSum> lock(generator);
		double sample = generator.getSample(t);
		return sample;
		return 0.;
	};

	SynthStream synth(sampleRate, 32, generateSample, generateSample);
	synth.play();

	MidiContext midiContext;

	sf::RenderWindow window(sf::VideoMode(1600, 1000), "Basic synth");
	setupGui(gui, window);
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);
	while (window.isOpen()) {
		static sf::Event event;
		static MidiEvent midiEvent;

		while (midiContext.pollEvent(midiEvent)) {
			gui.forwardEvent(midiEvent);
		}
		while (window.pollEvent(event)) {
			gui.forwardEvent(event);
		}

		window.clear(sf::Color::Black);
		window.draw(gui);
		window.display();
	}

	return 0;
}
