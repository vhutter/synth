#include "effects.h"

//DebugFilter::Impl::Impl(decltype(oscilloscope) oscilloscope, double maxSamp)
//	:oscilloscope(oscilloscope),
//	maxSamp(maxSamp)
//{
//}

DebugFilter::DebugFilter(GuiElement& gui)
	:impl{ std::make_shared<Impl>() }
{
	gui.addChildren({ impl->oscilloscope });
}

void DebugFilter::effectImpl(double t, double & sample)
{
	static auto sampleId = 0u;
	static std::vector<double> lastSamples(impl->oscilloscope->getResolution());
	lastSamples[sampleId++] = sample;
	if (sampleId == 500) {
		impl->oscilloscope->newSamples(lastSamples);
		sampleId = 0;
	}
	if (sample > impl->maxSamp) {
		impl->maxSamp = sample;
		//std::cout << maxSamp << "\r";
	}
}

EchoEffect::Impl::Impl(double coeff, unsigned bufSize)
	:coeff(coeff),
	bufSize(bufSize),
	echoBuf(bufSize, 0)
{
}

EchoEffect::EchoEffect(GuiElement & gui, unsigned sampleRate, double echoLength, double coeffArg)
	: impl(std::make_shared<Impl>(coeffArg, unsigned(sampleRate*echoLength)))
{
}

void EchoEffect::effectImpl(double t, double & sample)
{
	static unsigned sampleId{ 0 };
	unsigned idx = sampleId % impl->bufSize;
	sample += impl->echoBuf[idx] * impl->coeff;
	impl->echoBuf[idx] = sample;
	++sampleId;
}

VolumeControl::Impl::Impl(double lastTime, decltype(amp) amp, decltype(sliderVolume) sliderVolume)
	:lastTime(lastTime),
	amp(amp),
	sliderVolume(sliderVolume)
{
}

VolumeControl::VolumeControl(GuiElement & gui)
	:impl(std::make_shared<Impl>(0., 0.5, Slider::DefaultSlider("Volume", 0, 1, 30, 50, [this](const Slider& sliderVolume) {
		impl->amp.setValueLinear(sliderVolume.getValue(), impl->lastTime, 0.005);
	})))
{
	gui.addChildren({impl->sliderVolume});
}

void VolumeControl::effectImpl(double t, double & sample)
{
	impl->lastTime = t;
	sample *= impl->amp.getValue(t);
}
