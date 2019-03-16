#ifndef EFFECTS_H_DEFINED
#define EFFECTS_H_DEFINED

#include <memory>
#include <tuple>
#include "guiElements.h"
#include "generators.h"


template <typename ...T>
auto mergeEffects(T&& ...args)
{
	return[=](double t, double& sample) mutable
	{
		(args(t, sample), ...);
	};
}


template<class T>
class EffectBase
{
public:
	void operator()(double t, double& sample)
	{
		static_cast<T*>(this)->effectImpl(t, sample);
	}
	~EffectBase()
	{
		//auto _this = static_cast<T*>(this);
	}
};

class DebugFilter: public EffectBase<DebugFilter>
{
	friend class EffectBase<DebugFilter>;

public:
	DebugFilter(GuiElement& gui);
	void effectImpl(double t, double& sample);

private:
	struct Impl
	{
		std::shared_ptr<Oscilloscope> oscilloscope{ std::make_shared<Oscilloscope>(600, 50, 500, 200, 500, 1) };
		double maxSamp;
	};

	std::shared_ptr<Impl> impl;
};

class VolumeControl : public EffectBase<VolumeControl>
{
	friend class EffectBase<VolumeControl>;

public:
	VolumeControl(GuiElement& gui);
	void effectImpl(double t, double& sample);

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

class EchoEffect: public EffectBase<EchoEffect>
{
	friend class EffectBase<EchoEffect>;

public:
	EchoEffect(
		GuiElement& gui, 
		unsigned sampleRate, 
		double echoLength,
		double echoCoeff
	);
	void effectImpl(double t, double& sample);

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

// to be continued...
class Glider : public EffectBase<Glider>
{
	friend class EffectBase<Glider>;

public:
	Glider(GuiElement& gui);
	void effectImpl(double t, double& sample);

protected:
	struct Impl
	{
		std::atomic<double> lastTime, glideSpeed{ .5 };
		std::atomic<bool> glide{ false };
		std::shared_ptr<Slider> glideSpeedSlider{ Slider::DefaultSlider("Glide", 0, .5, 160, 50, glideSpeed) };
		std::shared_ptr<Button> glideButton{ Button::DefaultButton("Glide", 180, 180, glide) };
	};

	std::shared_ptr<Impl> impl;
};

//class Flanger : public EffectBase<Flanger>
//{
//	friend class EffectBase<Flanger>;
//
//public:
//	Flanger() {}
//	void effectImpl(double t, double& sample)
//	{
//		const double speed{ 99 };
//		const std::size_t bufSize{ 100 };
//		static std::vector<double> samples(bufSize, 0.);
//		static unsigned sampleId{ 0 };
//		unsigned idx = sampleId % bufSize;
//		samples[idx] = sample;
//		idx = std::floor(std::fmod(t*speed, bufSize));
//		sample = (sample + samples[idx]) / 2.;
//		++sampleId;
//	}
//
//protected:
//
//};

class TestFilter : public EffectBase<TestFilter>
{
	friend class EffectBase<TestFilter>;

public:
	TestFilter(GuiElement& gui)
		:impl{ std::make_shared<Impl>() }
	{ 
		gui.addChildren({ Slider::DefaultSlider("Glide", 0, 1, 200, 50, impl->a) });
		impl->b = 1.f - impl->a; impl->z = 0;
	};
	void effectImpl(double t, double& sample) 
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