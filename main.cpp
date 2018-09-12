#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <chrono>
#include <atomic>
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

	std::array<bool, 12> pressed = {0};
	std::atomic<unsigned> amp(std::numeric_limits<sf::Int16>::max());
	unsigned amp_max(std::numeric_limits<sf::Int16>::max());
	const unsigned sampleRate(44100);

	float rMax = 0.;

	SynthStream synth(sampleRate, 512, [&](double& t) -> double {
		float result = 0.;
		unsigned notesNumber = 0;
		for (int i=0; i<12; i++){
			if (pressed[i]) {
                const auto& f = notes[i].getFreq();
				result += waves::sine(t, 1.0, f);
				t = std::fmod(t, 2*M_PI*f);
                ++notesNumber;
			}
		}

		result = result / double(notesNumber) * amp;

		rMax = std::max(rMax, result);

		return result;
	});
	synth.play();
	synth.setVolume(100);

	sf::RenderWindow window(sf::VideoMode(800, 600), "Basic synth");
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);

    std::vector< std::shared_ptr<sf::Drawable> > objects = {
//        std::make_shared<SynthKeyboard> (SynthKeyboard(50, 100))
    };

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
                case sf::Event::KeyReleased:
                    updatePressed(pressed, event.key.code);
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::Space:
                            synth.resetTime();
                            break;
                        default:
                            break;
                    }
                    std::cout << rMax << "\n";
                    break;
                case sf::Event::MouseMoved:
                    break;
                case sf::Event::Resized:
                    window.setSize(sf::Vector2u(800, 600));
                    break;
                default:
                    break;
		    }
		}

//		window.clear(sf::Color::Black);

        for (auto& object: objects)
            window.draw(*object);

		window.display();
	}

	return 0;

}
