#ifndef EFFECTS_H_DEFINED
#define EFFECTS_H_DEFINED

#include <memory>
#include <tuple>
#include "guiElements.h"
#include "generators.h"


template <typename param_t, typename ...T>
auto mergeEffects(T&& ...args)
{
	return[=](double t, param_t& sample) mutable
	{
		(args(t, sample), ...);
	};
}


template<class T, class param_t>
class EffectBase
{
public:
	void operator()(double t, param_t& sample) const
	{
		static_cast<const T*>(this)->effectImpl(t, sample);
	}
};

class DebugFilter: public EffectBase<DebugFilter, double>
{
public:
	DebugFilter(GuiElement& gui);
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::shared_ptr<Oscilloscope> oscilloscope{ std::make_shared<Oscilloscope>(600, 50, 500, 200, 500, 1) };
		double maxSamp;
	};

	std::shared_ptr<Impl> impl;
};

class VolumeControl : public EffectBase<VolumeControl, double>
{
public:
	VolumeControl(GuiElement& gui);
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::atomic<double> lastTime{ 0. };
		ContinuousFunction amp{0.5};
		std::shared_ptr<Slider> sliderVolume = Slider::DefaultSlider("Volume", 0, 1, 30, 50, [this](const Slider& sliderVolume) {
			amp.setValueLinear(sliderVolume.getValue(), lastTime, 0.005);
		});
	};

	std::shared_ptr<Impl> impl;
};

class EchoEffect: public EffectBase<EchoEffect, double>
{
public:
	EchoEffect(
		GuiElement& gui, 
		unsigned sampleRate, 
		double echoLength,
		double echoCoeff
	);
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		const double coeff;
		const unsigned bufSize;
		std::vector<double> echoBuf;

		Impl(double coeff, unsigned bufSize);
	};

	std::shared_ptr<Impl> impl;
};

class Glider : public EffectBase<Glider, double>
{
public:
	Glider(
		GuiElement & gui, 
		const TimbreModel& model, 
		const std::vector<Note>& notes, 
		unsigned maxNotes
	);
	void onKeyEvent(unsigned keyIdx, SynthKey::State keyState);
	void effectImpl(double t, double & sample) const;

private:
	struct Impl
	{
		DynamicCompoundGenerator<Tone> glidingTone;
		ContinuousFunction glidePitch{ 100 };
		std::atomic<double> glideSpeed{ .5 }, lastTime{ 0. };
		std::atomic<bool> glide{ false };
		std::shared_ptr<Slider> glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, 160, 50, glideSpeed) };
		std::shared_ptr<Button> glideButton{ Button::DefaultButton("Glide", 180, 180, glide) };

		Impl(const TimbreModel& model) : glidingTone(model(100)) {}
	};

	unsigned lastPressed, maxNotes;
	const std::vector<Note>& notes;
	std::shared_ptr<Impl> impl;
};

template<class SampleGenerator_T>
class PitchBender:public EffectBase<PitchBender<SampleGenerator_T>, typename SampleGenerator_T::Base_t>
{
	using Base_t = DynamicToneSum::Base_t;

public:
	PitchBender(GuiElement& gui, SampleGenerator_T& generator)
		:generator(generator)
	{
		sliderPitch->setFixed(true);
		gui.addChildren({ sliderPitch });
	}
	void operator()(double t, Base_t& sample) const
	{
		generator.modifyMainPitch(
			generator.time(),
			generator.getMainFreq() + sliderPitch->getValue() * 1 / 9 * generator.getMainFreq()
		);
	}

private:

	SampleGenerator_T& generator;
	std::shared_ptr<Slider> sliderPitch{ Slider::DefaultSlider("Pitch", -1, 1, 100, 50) };

};

class TestFilter : public EffectBase<TestFilter, double>
{
public:
	TestFilter(GuiElement& gui)
		:impl{ std::make_shared<Impl>() }
	{ 
		gui.addChildren({ Slider::DefaultSlider("Glide", 0, 1, 200, 50, impl->a) });
		impl->b = 1.f - impl->a; impl->z = 0;
	};
	void effectImpl(double t, double& sample) const
	{ 
		sample = (sample * impl->b) + (impl->z * impl->a);
	}

protected:
	struct Impl
	{
		std::atomic<double> a{ 0.5 };
		double b, z;
	};

	std::shared_ptr<Impl> impl;
};


#endif // EFFECTS_H_DEFINED