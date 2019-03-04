#define _USE_MATH_DEFINES

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cmath>
#include <limits>
#include <chrono>
#include <utility>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <array>
#include <utility>

#include <iostream>

#include <frozen/unordered_map.h>
#include <frozen/string.h>

#include "tones.h"
#include "synth.h"
#include "generators.h"
#include "guiElements.h"
#include "gui.h"
#include "events.h"

int main()
{
	EmptyGuiElement gui{ {} };
	auto generator = CustomTone1<5>(gui, DebugFilter(gui));

	const auto& generateSample = [&](double t) -> double {
        std::lock_guard lock(generator);
		return generator.getSample(t);
	};

    const unsigned sampleRate(44100);
	SynthStream synth(sampleRate, 1, generateSample, generateSample);
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
