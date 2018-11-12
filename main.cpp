#include <SFML/Window.hpp>
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

#include "synth.h"
#include "generators.h"
#include "gui.h"


int main()
{
    std::cout << std::fixed << std::setprecision(2);

    const std::vector<Note> notes = {C, Cis, D, Dis, E, F, Fis, G, Gis, A, Ais, B};

    // Sound synthesis will be performed in a separate thread internally
    // The generateSample lambda function is created below for that reason
    std::mutex mtx;

	ContinuousFunction amp(0.5);
	ContinuousFunction currentPitch(0);
	std::atomic<bool> glide(false);
	std::atomic<double> glideSpeed(.3);
	std::atomic<double> octave(1.);
	double lastTime(0);

	const auto& toneEffect = [&](double t, Tone& tone) {
	    tone.phase += sin(t*200) * .9 / tone.note / M_PI/2;
	    tone.note = tone.note * octave;
    };

	std::vector<CompoundTone> tones;
	tones.reserve(12);
	for (unsigned i=0; i<12; ++i) {
        tones.emplace_back(CompoundTone(
            {
                Tone(notes[i], 1., waves::sawtooth, {toneEffect}),
                Tone(notes[i]*3, 0.3, waves::sine, {toneEffect})
            }
//            ADSREnvelope(.005, 0.35, 0.00, 0.2)
        ));
	}

	auto oscope = std::make_shared<Oscilloscope>(600, 50, 500, 200, 1000, 1);
	std::shared_ptr<Slider> sliderVolume(new Slider(Slider::DefaultSlider("Volume", 0,1, 30,50, [&](){
        std::lock_guard<std::mutex> lock(mtx);
        amp.setValueLinear(sliderVolume->getValue(), lastTime, 0.005);
    })));
	std::shared_ptr<Slider> sliderPitch(new Slider(Slider::DefaultSlider("Pitch", -1,1, 100,50, [&](){
        static double lastValue = 0;
        const double dif = sliderPitch->getValue() - lastValue;
        std::lock_guard<std::mutex> lock(mtx);
        lastValue = lastValue + dif;
        for(auto& tone: tones)
            tone.modifyMainPitch(lastTime, tone.getMainNote() + sliderPitch->getValue() * 1/9 * tone.getMainNote());
    })));
    sliderPitch->setFixed(true);
    std::shared_ptr<Slider> glideSpeedSlider(new Slider(Slider::DefaultSlider("Glide", 0,.5, 160, 50, glideSpeed)));
    std::shared_ptr<Button> glideButton(new Button(Button::DefaultButton("Glide", 180, 180, glide)));
	std::shared_ptr<SynthKeyboard> synthKeyboard = std::make_shared<SynthKeyboard>(50, 700, [&](unsigned keyIdx){
        static int lastIdx = -1;
        const auto& synth = synthKeyboard;
        std::lock_guard<std::mutex> lock(mtx);
        if (synth->isLastEventKeypress()) {
            if (glide && lastIdx != -1) {
                tones[lastIdx].envelope.stop(lastTime);
                currentPitch.setValueLinear(tones[keyIdx].getMainNote(), lastTime, glideSpeed);
            }
            tones[keyIdx].envelope.start(lastTime);
            (*synth)[keyIdx].setPressed(true);
            lastIdx = keyIdx;
        }
        else {
            tones[keyIdx].envelope.stop(lastTime);
            (*synth)[keyIdx].setPressed(false);
        }
    });
    std::vector< std::shared_ptr<GuiElement> > objects = {
        sliderVolume,
        oscope,
        synthKeyboard,
        sliderPitch,
        glideButton,
        glideSpeedSlider,
    };

	std::deque<double> lastSamples;
    const unsigned maxNotes = 4;
	const auto& generateSample = [&](double t) -> double {

        std::lock_guard<std::mutex> lock(mtx);
		double result = 0.;
		unsigned notesNumber = 0;
		for (unsigned i=0; i<tones.size(); i++){
			if (tones[i].envelope.isNonZero()) {
                if (++notesNumber > maxNotes) break;
                if(glide) tones[i].modifyMainPitch(t, currentPitch.getValue(t));
                double env = tones[i].envelope.getAmplitude(t);
				result += tones[i].getSample(t) * env;
			}
		}

		lastTime = t;
		result = result / double(maxNotes);
        lastSamples.push_back(result);

		return result  * amp.getValue(t);
	};

    const unsigned sampleRate(44100);
	SynthStream synth(sampleRate, 16, generateSample, generateSample);
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
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::MouseWheelScrolled: {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (event.mouseWheelScroll.delta > 0)
                        octave = octave*2.;
                    else
                        octave = octave*0.5;
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
