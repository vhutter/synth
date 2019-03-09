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
		auto _this = static_cast<T*>(this);
		if (_this->impl.unique()) {
			// delete it from the gui
		}
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

class VolumeControl : public EffectBase<VolumeControl>
{
	friend class EffectBase<VolumeControl>;

public:
	VolumeControl(GuiElement& gui);
	void effectImpl(double t, double& sample);

protected:
	struct Impl
	{
		std::atomic<double> lastTime;
		ContinuousFunction amp;
		std::shared_ptr<Slider> sliderVolume;

		Impl(double lastTime, decltype(amp) amp, decltype(sliderVolume) slider);
	};

	std::shared_ptr<Impl> impl;
};

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