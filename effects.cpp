#include "effects.h"
#include "gui/Button.h"
#include "gui/Slider.h"

DebugFilter::DebugFilter()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->oscilloscope->AABB();
	window->setSize(SynthVec2(aabb.width, aabb.height));
	window->setHeader(30, "Debug");
	window->getContentFrame()->addChild( impl->oscilloscope );
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

VolumeControl::VolumeControl()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->sliderVolume->AABB();
	window->setSize(SynthVec2(aabb.width, aabb.height));
	window->getContentFrame()->addChild( impl->sliderVolume );
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

EchoEffect::EchoEffect(unsigned sampleRate, double echoLength, double coeffArg)
	: impl(std::make_shared<Impl>(coeffArg, unsigned(sampleRate*echoLength)))
{
	// no window yet
}

void EchoEffect::effectImpl(double t, double & sample) const
{
	static unsigned sampleId{ 0 };
	unsigned idx = sampleId % impl->bufSize;
	sample += impl->echoBuf[idx] * impl->coeff;
	impl->echoBuf[idx] = sample;
	++sampleId;
}

Glider::Glider(const TimbreModel& model, const std::vector<Note>& notes, unsigned maxNotes)
	:maxNotes(maxNotes),
	notes{ notes },
	impl{ std::make_shared<Impl>(model) }
{
	auto aabbSlider = impl->glideSpeedSlider->AABB();
	auto aabbButton = impl->glideButton->AABB();
	window->setSize(SynthVec2(std::max(aabbSlider.width, aabbButton.width), aabbSlider.height + aabbButton.height));
	window->getContentFrame()->addChildAutoPos(impl->glideSpeedSlider);
	window->getContentFrame()->addChildAutoPos(impl->glideButton);
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
