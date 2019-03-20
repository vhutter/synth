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

void DebugFilter::effectImpl(double t, double & sample) const
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

VolumeControl::VolumeControl(GuiElement & gui)
	:impl{ std::make_shared<Impl>() }
{
	gui.addChildren({impl->sliderVolume});
}

void VolumeControl::effectImpl(double t, double & sample) const
{
	impl->lastTime = t;
	sample *= impl->amp.getValue(t);
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

void EchoEffect::effectImpl(double t, double & sample) const
{
	static unsigned sampleId{ 0 };
	unsigned idx = sampleId % impl->bufSize;
	sample += impl->echoBuf[idx] * impl->coeff;
	impl->echoBuf[idx] = sample;
	++sampleId;
}

Glider::Glider(GuiElement & gui, const TimbreModel& model, const std::vector<Note>& notes, unsigned maxNotes)
	:maxNotes(maxNotes),
	notes{ notes },
	impl{ std::make_shared<Impl>(model) }
{
	gui.addChildren({ impl->glideSpeedSlider, impl->glideButton });
}

void Glider::effectImpl(double t, double & sample) const
{
	if (impl->glide) {
		impl->glidingTone.modifyMainPitch(t, impl->glidePitch.getValue(t));
		sample = impl->glidingTone.getSample(t).value_or(0.) / maxNotes;
	}
	impl->lastTime = t;
}
void Glider::onKeyEvent(unsigned keyIdx, SynthKey::State keyState)
{
	if (keyState == SynthKey::State::Pressed) {
		impl->glidePitch.setValueLinear(notes[keyIdx], impl->lastTime, impl->glideSpeed);
		impl->glidingTone.start(impl->lastTime);
		lastPressed = keyIdx;
	}
	else if (keyIdx == lastPressed) {
		impl->glidingTone.stop(impl->lastTime);
	}
}
