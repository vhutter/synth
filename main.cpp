#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <chrono>
#include <mutex>
#include <iostream>
#include <memory>
#include <utility>
#include <deque>

#include "util.h"
#include "synth.h"
#include "generators.h"
#include "gui.h"

using namespace util;
const std::vector<Note> notes = {A, Ais, B, C, Cis, D, Dis, E, F, Fis, G, Gis};

int main()
{
    std::mutex mtx;

    const unsigned sampleRate(48000);
    const unsigned maxNotes = 4;

	unsigned amp(std::numeric_limits<sf::Int16>::max());
	double octave(1.);
	double lastTime(0);
	std::function<double(double,double,double)> waveGenerator = waves::sine;

	std::array<bool, 12> pressed = {0};
	std::array<ADSREnvelope, 12> envelopes;

	bool isSliderClicked = false;
	auto slider1 = std::make_shared<Slider>(Slider(100,300, Slider::Vertical));
	auto oscope1 = std::make_shared<Oscilloscope>(Oscilloscope(200, 300, 500, 200, 1000, 1));
	std::deque<double> lastSamples;


	const auto& generateSample = [&](double t) -> double {
        std::lock_guard<std::mutex> lock(mtx);
		double result = 0.;
		unsigned notesNumber = 0;
		for (int i=0; i<12; i++){
			if (pressed[i] || envelopes[i].isNonZero()) {
                if (++notesNumber > maxNotes) break;
                const auto& f = notes[i].getFreq() * octave;
                double env = envelopes[i].getAmplitude(t);
				result += waveGenerator(t, 1, f+slider1->getValue()) * env;
			}
		}

		lastTime = t;
		result = result / double(maxNotes);
        lastSamples.push_back(result*150);

		return result  * amp;
	};

	std::vector<double> samples(sampleRate);

	SynthStream synth(sampleRate, 512, generateSample);
	synth.play();
	synth.setVolume(100);

	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Basic synth");
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);

    std::vector< std::shared_ptr<sf::Drawable> > objects = {
        slider1,
        oscope1
    };
    SynthKeyboard kb(50, 700);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
		    switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::KeyPressed:
                case sf::Event::KeyReleased: {
                    std::unique_lock<std::mutex> lock(mtx);
                    // ^ reason for locking is the modification of
                    //      pressed
                    //      envelopes
                    //      waveGenerator
                    decltype(pressed) initiallyPressed(pressed);
                    updatePressed(pressed, event.key.code);
                    for(unsigned i=0; i<12; ++i) {
                        if (!initiallyPressed[i] && pressed[i]) {
                            envelopes[i].start(lastTime);
                            kb[i].setPressed(true);
                        }
                        else if (initiallyPressed[i] && !pressed[i]) {
                            envelopes[i].stop(lastTime);
                            kb[i].setPressed(false);
                        }
                    }
                    lock.unlock();

                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::Space:
                            // re-lock: this section will run rarely
                            // and its here only for testing anyway
                            lock.lock();
                            waveGenerator = waves::triangle;
                            lock.unlock();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    const auto& mousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                    if (slider1->containsPoint(mousePos)) {
                        isSliderClicked = true;
                    }
                    break;
                }
                case sf::Event::MouseButtonReleased: {
                    isSliderClicked = false;
                    break;
                }
                case sf::Event::MouseMoved: {
                    const auto& mousePos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
                    if (isSliderClicked) {
                        slider1->moveSlider(mousePos);
//                        std::cout << slider1->getValue() << "\n";
                    }
                    break;
                }
                case sf::Event::MouseWheelScrolled: {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (event.mouseWheelScroll.delta > 0)
                        octave = octave * 2.;
                    else
                        octave = octave / 2.;
                    break;
                }
                case sf::Event::Resized: {
                    sf::View view(sf::FloatRect(0, 0, event.size.width, event.size.height));
                    window.setView(view);
                    break;
                }
                default:
                    break;
		    }
		}

		window.clear(sf::Color::Black);

		std::unique_lock<std::mutex> lock(mtx);
		std::vector<double> ls(lastSamples.begin(), lastSamples.end());
		oscope1->newSamples(ls);
		lastSamples.clear();
		lock.unlock();
        for (auto& object: objects)
            window.draw(*object);

        window.draw(kb);

		window.display();
	}

	synth.stop();
	return 0;
}
