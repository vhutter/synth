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

VolumeControl::VolumeControl(GuiElement & gui)
	:impl{ std::make_shared<Impl>() }
{
	gui.addChildren({impl->sliderVolume});
}

void VolumeControl::effectImpl(double t, double & sample)
{
	impl->lastTime = t;
	sample *= impl->amp.getValue(t);
}

Glider::Glider(GuiElement & gui)
	:impl{std::make_shared<Impl>()}
{
	//TimbreModel glidingToneModel{ myToneModel };
	//glidingToneModel.before = [this](double t, CompoundGenerator<Tone>& input) {
	//	input.modifyMainPitch(t, glidePitch.getValue(t));
	//};
	//static CompoundTone glidingTone{ glidingToneModel(notes.front()) };
	//myToneModel.after = [this](double t, double& sample) {
	//	if (glide) {
	//		glidingTone.modifyMainPitch(t, glidePitch.getValue(t));
	//		sample = glidingTone.getSample(t);
	//	}
	//};
	gui.addChildren({ impl->glideSpeedSlider, impl->glideButton });
}

void Glider::effectImpl(double t, double & sample)
{
	//if (impl->glide) {
	//	impl->glidingTone.modifyMainPitch(t, glidePitch.getValue(t));
	//	sample = glidingTone.getSample(t);
	//}
}
