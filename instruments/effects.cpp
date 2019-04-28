#include "effects.h"
#include "../gui/Button.h"
#include "../gui/Slider.h"
#include "../gui/SynthKeyboard.h"

#include <bitset>
#include <sstream>

DebugEffect::DebugEffect()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->oscilloscope->AABB();
	frame->setSize(SynthVec2(aabb.width+20, aabb.height+40));
	impl->maxSampText = TextDisplay::DefaultText("0                  \n", 20);
	impl->eventText = TextDisplay::DefaultText(getMidiEventInfo(MidiEvent()), 20);
	addToggleButton();
	frame->setBgColor(sf::Color(0x222222cc));

	frame->addChildAutoPos( impl->oscilloscope );
	frame->addChildAutoPos( TextDisplay::DefaultText("Max sample:", 20) );
	frame->addChildAutoPos( impl->maxSampText );
	frame->addChildAutoPos( impl->eventText );
	frame->addChild(std::make_unique<EmptyGuiElement>([impl = this->impl](const MidiEvent & event) {
		impl->eventText->setText(getMidiEventInfo(event));
	}));

	frame->fitToChildren();
}

void DebugEffect::effectImpl(double t, double & sample) const
{
	static auto sampleId = 0u;
	auto& lastSamples = impl->lastSamples;
	lastSamples[sampleId++] = sample;
	if (sampleId == 500) {
		impl->oscilloscope->newSamples(lastSamples);
		sampleId = 0;
		impl->maxSampText->setText(std::to_string(impl->maxSamp));
		impl->maxSamp = 0;
	}
	if (sample > impl->maxSamp) {
		impl->maxSamp = sample;
	}
}

std::string DebugEffect::getMidiEventInfo(const MidiEvent& event)
{
	std::ostringstream output;
	output << 
		"Max. wheel value: " << MidiEvent::wheelValueMax() << "\n" <<
		"Current wheel value: " << event.getWheelValue() << "\n" <<
		"Event type: " << std::bitset<8>(uint8_t(event.getType())) << "\n" <<
		"Key code: " << unsigned(event.getKey()) << "\n" <<
		"Velocity: " << unsigned(event.getVelocity());
	return output.str();
}

VolumeControl::VolumeControl()
	:impl{ std::make_shared<Impl>() }
{
	auto aabb = impl->sliderVolume->AABB();
	frame->addChildAutoPos( impl->sliderVolume );
	frame->fitToChildren();
}

void VolumeControl::effectImpl(double t, double & sample) const
{
	impl->lastTime = t;
	sample *= impl->amp.getValue(t);
}

DelayEffect::DelayEffect(unsigned sampleRate, double echoLength, double coeffArg)
	:impl{ std::make_shared<Impl>() }
{
	auto& _impl = *impl;
	
	_impl.coeff = coeffArg;
	_impl.length = echoLength;
	_impl.sampleRate = sampleRate;
	_impl.echoBuf = std::vector<double>(unsigned(sampleRate * echoLength), 0);
	_impl.sliderCoeff = Slider::DefaultSlider("Intensity", 0, 1, _impl.coeff);
	_impl.sliderTime = Slider::DefaultSlider("Time", 0.02, echoLength, _impl.length);

	auto aabbCoeff = impl->sliderCoeff->AABB();
	setWidth(aabbCoeff.width * 4);
	frame->setChildAlignment(5);
	frame->addChildAutoPos(impl->sliderCoeff);
	frame->addChildAutoPos(_impl.sliderTime);
	addToggleButton();
	frame->fitToChildren();

	frame->setBgColor(sf::Color::Black);
}

void DelayEffect::effectImpl(double t, double & sample) const
{
	auto& _impl = *impl;
	static unsigned sampleId{ 0 };
	unsigned idx = sampleId % unsigned(_impl.sampleRate * _impl.length);
	sample += _impl.echoBuf[idx] * _impl.coeff;
	_impl.echoBuf[idx] = sample;
	++sampleId;
}

Glider::Impl::Impl(const TimbreModel& model) 
	:glidingTone(model(1))
{
}

Glider::Glider(const TimbreModel& model, const std::vector<Note>& notes, unsigned maxNotes)
	:maxNotes(maxNotes),
	notes( notes ),
	impl( std::make_shared<Impl>(model) )
{
	auto aabbSlider = impl->glideSpeedSlider->AABB();
	frame->setSize(SynthVec2(aabbSlider.width, aabbSlider.height));
	addToggleButton();
	frame->addChildAutoPos(impl->glideSpeedSlider);
	frame->fitToChildren();
}

void Glider::effectImpl(double t, double & sample) const
{
	impl->glidingTone.modifyMainPitch(t, impl->glidePitch.getValue(t));
	sample = impl->glidingTone.getSample(t).value_or(0.) / maxNotes;
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