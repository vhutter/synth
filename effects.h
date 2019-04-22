#ifndef EFFECTS_H_DEFINED
#define EFFECTS_H_DEFINED

#include <memory>
#include <tuple>
#include "generators.h"
#include "gui/Slider.h"
#include "gui/Button.h"
#include "gui/Oscilloscope.h"
#include "gui/Window.h"


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
	EffectBase()
		:window(std::make_shared<Window>(0,0,sf::Color(0x222222aa))) // slightly transparent gray background
	{}

	void operator()(double t, param_t& sample) const
	{
		static_cast<const T*>(this)->effectImpl(t, sample);
	}

	const std::shared_ptr<Window> getWindow() const
	{
		return window;
	}

protected:
	std::shared_ptr<Window> window;
};

template<class T>
using AfterEffectBase = EffectBase<T, double>;

class DebugFilter: public EffectBase<DebugFilter, double>
{
public:
	DebugFilter();
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::shared_ptr<Oscilloscope> oscilloscope{ std::make_shared<Oscilloscope>(500, 200, 500, 1) };
		double maxSamp;
	};

	std::shared_ptr<Impl> impl;
};

class VolumeControl : public EffectBase<VolumeControl, double>
{
public:
	VolumeControl();
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::atomic<double> lastTime{ 0. };
		ContinuousFunction amp{0.5};
		std::shared_ptr<Slider> sliderVolume = Slider::DefaultSlider("Volume", 0, 1, [this](const Slider& sliderVolume) {
			amp.setValueLinear(sliderVolume.getValue(), lastTime, 0.005);
		});
	};

	std::shared_ptr<Impl> impl;
};

class EchoEffect: public EffectBase<EchoEffect, double>
{
public:
	EchoEffect( 
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
		std::shared_ptr<Slider> glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, glideSpeed) };
		std::shared_ptr<Button> glideButton{ Button::DefaultButton("Glide", glide) };

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
	PitchBender(SampleGenerator_T& generator)
		:generator(generator)
	{
		auto aabb = sliderPitch->AABB();
		const auto& window = EffectBase<PitchBender<SampleGenerator_T>, typename SampleGenerator_T::Base_t>::window;
		window->setSize(SynthVec2(aabb.width, aabb.height));
		window->getContentFrame()->addChild( sliderPitch );
		sliderPitch->setFixed(true);
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
	std::shared_ptr<Slider> sliderPitch{ Slider::DefaultSlider("Pitch", -1, 1) };

};

//class TestFilter : public EffectBase<TestFilter, double>
//{
//public:
//	TestFilter()
//		:impl{ std::make_shared<Impl>() }
//	{ 
//		window->getContentFrame->addChild( Slider::DefaultSlider("Glide", 0, 1, impl->a) );
//		impl->b = 1.f - impl->a; impl->z = 0;
//	};
//	void effectImpl(double t, double& sample) const
//	{ 
//		sample = (sample * impl->b) + (impl->z * impl->a);
//	}
//
//protected:
//	struct Impl
//	{
//		std::atomic<double> a{ 0.5 };
//		double b, z;
//	};
//
//	std::shared_ptr<Impl> impl;
//};


#endif // EFFECTS_H_DEFINED