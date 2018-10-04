#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <chrono>
#include <mutex>
#include <memory>
#include <utility>
#include <deque>

#include <iostream>
#include <iomanip>

#include "util.h"
#include "synth.h"
#include "generators.h"
#include "gui.h"

using namespace util;
const std::vector<Note> notes = {C, Cis, D, Dis, E, F, Fis, G, Gis, A, Ais, B};

int main()
{
    std::cout << std::fixed << std::setprecision(2);

    // Sound synthesis will be performed in a separate thread internally
    // The generateSample lambda function is created below for that reason
    std::mutex mtx;

	double amp(1);
	double octave(1);
	double lastTime(0);

	std::array<waves::wave_t, 4> waveGenerators = {waves::sawtooth, waves::square, waves::triangle, waves::sine};
    unsigned generatorIdx = 3; // sine

	std::array<bool, 12> pressed = {0};
	std::array<ADSREnvelope, 12> envelopes;
	std::array<double, 12> freqs;
	std::array<double, 12> phases = {0};
	std::copy(notes.begin(), notes.end(), freqs.begin());

	std::shared_ptr<Slider> sliderVolume = std::make_shared<Slider>("Volume", 100,300, Slider::Vertical, [](){});
	std::shared_ptr<Slider> sliderPitch  = std::make_shared<Slider>("Pitch", 200,300, Slider::Vertical, [&](){
        std::unique_lock<std::mutex> lock(mtx);
        for (unsigned i=0; i<notes.size(); ++i) {
            {
                double f1 = freqs[i];
                double f2 = notes[i] + notes[i]/8*sliderPitch->getValue(); // major second
                const auto& t = lastTime;

                double p = (t+phases[i]) * f1 / f2 - t;
                phases[i] = p;
                freqs[i] = f2;
            }
        }
    });
    sliderPitch->setFixed(true);
	auto oscope = std::make_shared<Oscilloscope>(300, 300, 500, 200, 1000, 1);
	std::shared_ptr<SynthKeyboard> synthKeyboard = std::make_shared<SynthKeyboard>(50, 700, [&](unsigned keyIdx){
        const auto& synth = synthKeyboard;
        std::lock_guard<std::mutex> lock(mtx);
        if (synth->isLastEventKeypress()) {
            envelopes[keyIdx].start(lastTime);
            (*synth)[keyIdx].setPressed(true);
        }
        else {
            envelopes[keyIdx].stop(lastTime);
            (*synth)[keyIdx].setPressed(false);
        }
    });
	std::deque<double> lastSamples;

    std::vector< std::shared_ptr<GuiElement> > objects = {
        sliderVolume,
        oscope,
        synthKeyboard,
        sliderPitch,
    };

    const unsigned maxNotes = 4;
	const auto& generateSample = [&](double t) -> double {

        std::lock_guard<std::mutex> lock(mtx);
		double result = 0.;
		unsigned notesNumber = 0;
		for (unsigned i=0; i<pressed.size(); i++){
			if ((*synthKeyboard)[i].isPressed() || envelopes[i].isNonZero()) {
                if (++notesNumber > maxNotes) break;
                const auto& f = freqs[i];
                double env = envelopes[i].getAmplitude(t);
				result += waveGenerators[generatorIdx](t, 1, f * octave, phases[i]) * env;
			}
		}

		lastTime = t;
		result = result / double(maxNotes);
        lastSamples.push_back(result*150);
        amp = (1+sliderVolume->getValue())/2;

		return result  * amp;
	};


    const unsigned sampleRate(42100);
	SynthStream synth(sampleRate, 32, generateSample);
	synth.play();

	sf::RenderWindow window(sf::VideoMode(1400, 1000), "Basic synth");
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
            for (auto& object: objects)
                object->forwardEvent(event);

		    switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::KeyPressed: {
                    switch (event.key.code) {
                        case sf::Keyboard::Escape: {
                            window.close();
                            break;
                        }
                        case sf::Keyboard::Space: {
                            std::lock_guard<std::mutex> lock(mtx);
                            generatorIdx = (generatorIdx+1) % 4;
                            break;
                        }
                        default:
                            break;
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

		std::cout << freqs[0] * octave << "       \r";

		window.clear(sf::Color::Black);

		std::unique_lock<std::mutex> lock(mtx);
		std::vector<double> ls(lastSamples.begin(), lastSamples.end());
		oscope->newSamples(ls);
		lastSamples.clear();
		lock.unlock();

        for (auto& object: objects)
            window.draw(*object);

		window.display();
	}

	synth.stop();
	return 0;
}
