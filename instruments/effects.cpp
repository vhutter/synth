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

DelayEffect::DelayEffect(unsigned sampleRate, double echoLength, double coeffArg)
	:impl{ std::make_shared<Impl>() }
{
	impl->coeff = coeffArg;
	impl->length = echoLength;
	impl->sampleRate = sampleRate;
	impl->echoBuf = std::vector<double>(unsigned(sampleRate * echoLength), 0);
	impl->sampleRate = sampleRate;

	auto aabbCoeff = impl->sliderCoeff->AABB();
	frame->setSize(SynthVec2(aabbCoeff.width*2, aabbCoeff.height+40));
	frame->addChildAutoPos(impl->sliderCoeff);
	impl->sliderTime = Slider::DefaultSlider("Time", 0, echoLength, impl->length);
	frame->addChildAutoPos(impl->sliderTime);
	impl->effectOn = Button::DefaultButton("Off", [this]() {
		impl->on = !impl->on;
		impl->echoBuf.assign(impl->echoBuf.size(), 0.);
		impl->effectOn->setText(impl->on ? "On" : "Off");
	});
	impl->effectOn->setFixedSize(true);
	frame->addChildAutoPos(impl->effectOn);

	frame->setBgColor(sf::Color::Black);
}

void DelayEffect::effectImpl(double t, double & sample) const
{
	if (impl->on) {
		static unsigned sampleId{ 0 };
		unsigned idx = sampleId % unsigned(impl->sampleRate * impl->length);
		sample += impl->echoBuf[idx] * impl->coeff;
		impl->echoBuf[idx] = sample;
		++sampleId;
	}
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
	notes( notes ),
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
		//std::cout << sample << "\n";
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