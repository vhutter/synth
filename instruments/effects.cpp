#include "effects.h"
#include "../gui/Button.h"
#include "../gui/Slider.h"
#include "../gui/SynthKeyboard.h"

DebugFilter::DebugFilter()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->oscilloscope->AABB();
	frame->setSize(SynthVec2(aabb.width+20, aabb.height+40));
	frame->addChildAutoPos( impl->oscilloscope );
	frame->addChildAutoPos( TextDisplay::DefaultText("Max sample:", 20) );
	impl->maxSampText = TextDisplay::DefaultText("0       ", 20);
	frame->addChildAutoPos( impl->maxSampText );
	frame->setBgColor(sf::Color(0x222222cc));
}

void DebugFilter::effectImpl(double t, double & sample) const
{
	static auto sampleId = 0u;
	static std::vector<double> lastSamples(impl->oscilloscope->getResolution());
	lastSamples[sampleId++] = sample;
	if (sampleId == 500) {
		impl->oscilloscope->newSamples(lastSamples);
		sampleId = 0;
		impl->maxSamp = 0;
	}
	if (sample > impl->maxSamp) {
		impl->maxSamp = sample;
		impl->maxSampText->setText(std::to_string(impl->maxSamp));
	}
}

VolumeControl::VolumeControl()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->sliderVolume->AABB();
	frame->setSize(SynthVec2(aabb.width, aabb.height));
	frame->addChild( impl->sliderVolume );
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

Glider::Impl::Impl(const TimbreModel& model) :
	glidingTone(model(100)),
	glideButton{ Button::DefaultButton("Off", [this]() {
		glide = !glide;
		if (glide) glideButton->setText("On");
		else glideButton->setText("Off");
}) }
{
	glideButton->setFixedSize(true);
}

Glider::Glider(const TimbreModel& model, const std::vector<Note>& notes, unsigned maxNotes)
	:maxNotes(maxNotes),
	notes{ notes },
	impl( std::make_shared<Impl>(model) )
{
	auto aabbSlider = impl->glideSpeedSlider->AABB();
	auto aabbButton = impl->glideButton->AABB();
	frame->setSize(SynthVec2(std::max(aabbSlider.width, aabbButton.width), aabbSlider.height + aabbButton.height));
	frame->addChildAutoPos(impl->glideSpeedSlider);
	frame->addChildAutoPos(impl->glideButton);
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