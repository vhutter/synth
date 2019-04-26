#ifndef EFFECTS_H_DEFINED
#define EFFECTS_H_DEFINED

#include <memory>
#include <tuple>
#include "generators.h"
#include "../gui/Slider.h"
#include "../gui/Button.h"
#include "../gui/Oscilloscope.h"
#include "../gui/Window.h"


template<class T, class param_t>
class EffectBase
{
public:
	EffectBase()
		:frame{ std::make_shared<Frame>() }
	{
		frame->setBgColor(sf::Color(0x555555aa));
		frame->setSize(SynthVec2(300, 300));
	}

	void operator()(double t, param_t& sample) const
	{
		if (*active) {
			static_cast<const T*>(this)->effectImpl(t, sample);
		}
	}

	const std::shared_ptr<Frame> getFrame() const
	{
		return frame;
	}

protected:
	std::shared_ptr<Frame> frame;

	void addToggleButton()
	{
		*active = false;
		std::shared_ptr<Button> activateButton{ Button::OnOffButton(*active) };
		frame->addChildAutoPos(activateButton);
	}

	void setWidth(unsigned width)
	{
		frame->setSize(SynthVec2(width, 0));
	}

private:
	std::shared_ptr< std::atomic<bool> > active{ std::make_shared<std::atomic<bool>>(true) };
};

template<class Effect_t>
using AfterEffectBase = EffectBase<Effect_t, double>;

class DebugFilter: public AfterEffectBase<DebugFilter>
{
public:
	DebugFilter();
	void effectImpl(double t, double& sample) const;

private:

	struct Impl
	{
		std::shared_ptr<Oscilloscope> oscilloscope{ std::make_shared<Oscilloscope>(500, 200, 500) };
		std::shared_ptr<TextDisplay> maxSampText;
		double maxSamp;
	};

	std::shared_ptr<Impl> impl;
};

class VolumeControl : public AfterEffectBase<VolumeControl>
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

class DelayEffect: public AfterEffectBase<DelayEffect>
{
public:
	DelayEffect( 
		unsigned sampleRate, 
		double echoLength,
		double echoCoeff
	);
	void effectImpl(double t, double& sample) const;

private:
	struct Impl
	{
		std::atomic<double> coeff;
		std::atomic<double> length;
		unsigned sampleRate;
		std::vector<double> echoBuf;
		std::shared_ptr<Slider> sliderCoeff;
		std::shared_ptr<Slider> sliderTime;
	};

	std::shared_ptr<Impl> impl;
};

class Glider : public AfterEffectBase<Glider>
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
		std::shared_ptr<Slider> glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, glideSpeed) };

		Impl(const TimbreModel& model);
	};

	unsigned lastPressed, maxNotes;
	std::vector<Note> notes;
	std::shared_ptr<Impl> impl;
};

template<class SampleGenerator_T>
class PitchBender:public EffectBase<PitchBender<SampleGenerator_T>, typename SampleGenerator_T>
{
	using base = EffectBase<PitchBender<SampleGenerator_T>, SampleGenerator_T>;
public:
	PitchBender()
		:base()
	{
		auto aabb = sliderPitch->AABB();
		const auto& frame = base::frame;
		base::addToggleButton();
		base::setWidth(aabb.width);
		frame->addChildAutoPos( sliderPitch );
		frame->fitToChildren();
		sliderPitch->setFixed(true);
	}
	void effectImpl(double t, SampleGenerator_T& generator) const
	{
		generator.modifyMainPitch(
			generator.time(),
			generator.getMainFreq() + sliderPitch->getValue() * 1 / 9 * generator.getMainFreq()
		);
	}

private:
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