#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <chrono>
#include <atomic>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <memory>
#include <utility>

#include "util.h"
#include "synth.h"
#include "generators.h"
#include "gui.h"

using namespace utility;
const std::vector<Note> notes = {A, Ais, B, C, Cis, D, Dis, E, F, Fis, G, Gis};

int main()
{

    const unsigned sampleRate(44100);
    const unsigned maxNotes = 4;

    /// TODO:
    /// should handle data between threads with one mutex instead of multiple atomics

	std::atomic<unsigned> amp(std::numeric_limits<sf::Int16>::max());
	std::atomic<double> octave(1.);
	std::atomic<double> lastTime(0);
	std::function<double(double,double,double)> waveGenerator = waves::sawtooth;

	std::array<bool, 12> pressed = {0};
	std::array<ADSREnvelope, 12> envelopes;

	SynthStream synth(sampleRate, 512, [&](double t) -> double {
		float result = 0.;
		unsigned notesNumber = 0;
		for (int i=0; i<12; i++){
			if (pressed[i] || envelopes[i].isNonZero()) {
                if (++notesNumber > maxNotes) break;
                const auto& f = notes[i].getFreq() * octave;
                double env = envelopes[i].getAmplitude(t);
				result += waveGenerator(t, 1., f) * env;
			}
		}

		lastTime = t;
		result = result / double(maxNotes) * amp;

		return result;
	});
	synth.play();
	synth.setVolume(100);

	sf::RenderWindow window(sf::VideoMode(800, 600), "Basic synth");
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);

    std::vector< std::shared_ptr<sf::Drawable> > objects = {};
    SynthKeyboard kb(50, 100);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
		    switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                case sf::Event::KeyReleased: {
                    decltype(pressed) initiallyPressed(pressed);
                    updatePressed(pressed, event.key.code);
                    for(unsigned i=0; i<12; ++i)
                        if (!initiallyPressed[i] && pressed[i]) {
                            envelopes[i].start(lastTime);
                            kb[i].setPressed(true);
                        }
                        else if (initiallyPressed[i] && !pressed[i]) {
                            envelopes[i].stop(lastTime);
                            kb[i].setPressed(false);
                        }

                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::Space:
                            waveGenerator = waves::triangle;
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::MouseMoved:
                    break;
                case sf::Event::MouseWheelScrolled:
                    if (event.mouseWheelScroll.delta > 0)
                        octave = octave * 2.;
                    else
                        octave = octave / 2.;
                    break;
                case sf::Event::Resized:
                    window.setSize(sf::Vector2u(800, 600));
                    break;
                default:
                    break;
		    }
		}

		window.clear(sf::Color::Black);

        for (auto& object: objects)
            window.draw(*object);

        window.draw(kb);

		window.display();
	}

	return 0;

}
