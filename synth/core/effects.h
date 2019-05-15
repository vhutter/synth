#ifndef EFFECTS_H_DEFINED
#define EFFECTS_H_DEFINED

#include <string>
#include <memory>
#include <tuple>
#include "generators.h"
#include "AudioFile/AudioFile.h"
#include "../gui/Slider.h"
#include "../gui/Button.h"
#include "../gui/Oscilloscope.h"
#include "../gui/Window.h"
#include "../gui/events.h"
#include "../utility.h"

class EffectBase
{
public:
	EffectBase()
		:frame{ std::make_shared<Frame>() },
		configFrame{ std::make_shared<Frame>() }
	{
		frame->setBgColor(sf::Color(getConfig("effectBgColor")));
		frame->setSize(SynthVec2(3000, 3000));
		frame->setFocusable(false);

		configFrame->setBgColor(sf::Color::Black);
	}

	const std::shared_ptr<Frame> getFrame() const { return frame; }
	const std::shared_ptr<Frame> getConfigFrame() const { return configFrame; }
	bool isActive() const { return *active; }

protected:
	void addToggleButton()
	{
		*active = false;
		std::shared_ptr<Button> activateButton{ Button::OnOffButton(*active) };
		frame->addChildAutoPos(activateButton);
	}
	void setWidth(unsigned width) { frame->setSize(SynthVec2(width, 0)); }

	std::shared_ptr<Frame> frame, configFrame;

private:
	std::shared_ptr< std::atomic<bool> > active{ std::make_shared<std::atomic<bool>>(true) };
};

template<class T, class param_t>
class PerSampleEffectBase : public EffectBase
{
public:
	void operator()(double t, param_t& sample) const
	{
		if (isActive()) {
			static_cast<const T*>(this)->effectImpl(t, sample);
		}
	}
};

template<class Effect_t>
using PostSampleEffect = PerSampleEffectBase<Effect_t, double>;

class DebugEffect: public PostSampleEffect<DebugEffect>
{
public:
	DebugEffect();
	void effectImpl(double t, double& sample) const;

private:

	static std::string getMidiEventInfo(const MidiEvent& event);

	struct Impl
	{
		const unsigned resolution = 500;
		std::shared_ptr<Oscilloscope> oscilloscope{ std::make_shared<Oscilloscope>(500, 200, resolution) };
		std::shared_ptr<TextDisplay> maxSampText, eventText;
		std::vector<double> lastSamples = std::vector<double>( resolution, 0. );
		double maxSamp;
		unsigned sampleId{ 0 };
	};

	std::shared_ptr<Impl> impl;
};

class VolumeControl : public PostSampleEffect<VolumeControl>
{
public:
	VolumeControl();
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::atomic<double> lastTime{ 0. };
		ContinuousFunction amp;
		std::shared_ptr<Slider> sliderVolume = Slider::DefaultSlider("Volume", 0, 1, [this](const Slider& sliderVolume) {
			amp.setValueLinear(sliderVolume.getValue(), lastTime, 0.005);
		});
	};

	std::shared_ptr<Impl> impl;
};

class DelayEffect: public PostSampleEffect<DelayEffect>
{
public:
	DelayEffect( 
		unsigned sampleRate, 
		double echoLength,
		double echoCoeff,
		unsigned nChannels = 2
	);
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::atomic<double> coeff;
		std::atomic<double> length;
		unsigned sampleRate, sampleId{ 0 };
		std::vector<double> echoBuf;
		std::shared_ptr<Slider> sliderCoeff;
		std::shared_ptr<Slider> sliderTime;
	};

	std::shared_ptr<Impl> impl;
	const unsigned nChannels;
};

class Glider : public PostSampleEffect<Glider>
{
public:
	Glider(
		const TimbreModel& model, 
		const std::vector<Note>& notes, 
		unsigned maxNotes
	);
	void onKeyEvent(unsigned keyIdx, SynthKey::State keyState);
	void effectImpl(double t, double & sample) const;

private:
	struct Impl
	{
		Dynamic<Composite<WaveGenerator>> glidingTone;
		ContinuousFunction glidePitch{ 100 };
		std::atomic<double> glideSpeed{ .5 }, lastTime{ 0. };
		std::shared_ptr<Slider> glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, glideSpeed) };

		Impl(const TimbreModel& model);
	};

	unsigned lastPressed, maxNotes;
	std::vector<Note> notes;
	std::shared_ptr<Impl> impl;
};

class SaveToFile : public PostSampleEffect<SaveToFile>
{
public:

	SaveToFile(
		const std::string& fname,
		unsigned sampleRate,
		unsigned channels
	);

	void effectImpl(double t, double& sample) const;

	void start(){impl->isOn = true;impl->start();}
	void stop() {impl->isOn = false;impl->stop();}


private:
	struct Impl
	{
		const std::string dirName{"Records"};
		std::string fname;
		AudioFile<double>::AudioBuffer buffer;
		unsigned channelId = 0, sampleRate, channels;
		std::atomic<bool> isOn{ false };
		std::mutex mtx; // fname, sampleId, sampleRate may change from other threads
		std::shared_ptr<TextDisplay> displayResult;

		void start();
		void stop();

		std::shared_ptr<Button> onOff{ Button::OnOffButton(isOn, [this](bool on) {
			std::lock_guard lock(mtx);
			if (on) {
				start();
			}
			else {
				stop();
			}
		}) };
	};

	std::shared_ptr<Impl> impl;
};

template<class SampleGenerator_T>
class PitchBender : public EffectBase
{
public:
	PitchBender(SampleGenerator_T& gen)
		:EffectBase(),
		generator(gen)
	{
		auto aabb = sliderPitch->AABB();
		const auto& frame = getFrame();
		addToggleButton();
		setWidth(aabb.width);
		frame->addChildAutoPos( sliderPitch );
		frame->fitToChildren();
		sliderPitch->setFixed(true);

		frame->addChild(std::make_unique<EmptyGuiElement>([sliderPitch = this->sliderPitch](const MidiEvent & event) {
			if (event.getType() == MidiEvent::Type::WHEEL) {
				sliderPitch->setValue(event.getWheelValueNorm() * 2. - 1);
			}
		}));

		configFrame->addChildAutoPos(std::make_unique<TextDisplay>("Pitch bend settings", 0, getConfig("defaultTextHeight"), 16));
		configFrame->addChildAutoPos(sliderPitch->getConfigFrame());
		configFrame->fitToChildren();
	}

private:
	SampleGenerator_T& generator;
	std::shared_ptr<Slider> sliderPitch{ Slider::DefaultSlider("Pitch", -1, 1, [this](const Slider & sliderPitch) {
		if (isActive()) {
			std::lock_guard lock(generator);
			//generator.q = std::pow(2., sliderPitch.getValue());
			generator.modifyMainPitch(
				generator.time(),
				generator.getMainFreq() + sliderPitch.getValue() * 1 / 9 * generator.getMainFreq()
			);
		}
	}) };
};



#endif // EFFECTS_H_DEFINED