#include "effects.h"
#include "../gui/Button.h"
#include "../gui/Slider.h"
#include "../gui/SynthKeyboard.h"

#include <bitset>
#include <sstream>
#include <filesystem>

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
	auto& _impl = *impl;
	auto& lastSamples = _impl.lastSamples;
	lastSamples[_impl.sampleId++] = sample;
	if (_impl.sampleId == 500) {
		_impl.oscilloscope->newSamples(lastSamples);
		_impl.sampleId = 0;
		_impl.maxSampText->setText(std::to_string(impl->maxSamp));
		_impl.maxSamp = 0;
	}
	if (sample > _impl.maxSamp) {
		_impl.maxSamp = sample;
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
	impl->sliderVolume->setValue(1);
	auto aabb = impl->sliderVolume->AABB();
	frame->addChildAutoPos( impl->sliderVolume );
	frame->fitToChildren();

	configFrame->addChildAutoPos(std::make_unique<TextDisplay>("Volume settings", 0, 30, 16));
	configFrame->addChildAutoPos(impl->sliderVolume->getConfigFrame());
	configFrame->fitToChildren();
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

	configFrame->addChildAutoPos(std::make_unique<TextDisplay>("Delay settings", 0, 30, 16));
	configFrame->addChildAutoPos(_impl.sliderCoeff->getConfigFrame());
	configFrame->addChildAutoPos(_impl.sliderTime->getConfigFrame());
	configFrame->fitToChildren();
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

	configFrame->addChildAutoPos(std::make_unique<TextDisplay>("Glider settings", 0, 30, 16));
	configFrame->addChildAutoPos(impl->glideSpeedSlider->getConfigFrame());
	configFrame->fitToChildren();
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

SaveToFile::SaveToFile(
	const std::string& fname,
	unsigned sampleRate,
	unsigned channels)
	:impl( std::make_shared<Impl>() )
{
	impl->fname = fname;
	impl->sampleRate = sampleRate;
	impl->channels = channels;
	
	auto inputField = std::make_shared<InputField>(InputField::Alpha, 150, 30);
	inputField->setOnEnd([impl = this->impl, inputField]() {
		impl->fname = inputField->getText();
	});

	impl->displayResult = TextDisplay::DefaultText(" ", 14);

	// Creating the directory to put records in
	namespace fs = std::filesystem;

	bool created = false;
	if (fs::exists(impl->dirName))
		created = true;
	else {
		try {
			created = fs::create_directory(impl->dirName);
		}
		catch (...) {}
	}

	if (created) {
		impl->displayResult->setText(impl->dirName+" directory created.");
	}
	else {
		impl->displayResult->setText("Unable to access "s+impl->dirName+" directory."s);
	}

	// Default file name


	frame->setChildAlignment(15);
	frame->addChildAutoPos(impl->onOff);
	frame->newLine();
	frame->addChildAutoPos(TextDisplay::DefaultText("File name: ", 14));
	frame->addChildAutoPos(inputField);
	frame->newLine();
	frame->addChildAutoPos(impl->displayResult);
	frame->setSize({450, 200});
}

void SaveToFile::effectImpl(double t, double& sample) const
{
	auto& _impl = *impl;
	if (_impl.isOn) {
		std::lock_guard lock(_impl.mtx);
		_impl.buffer[_impl.channelId].push_back(sample);
		_impl.channelId = (_impl.channelId + 1) % _impl.channels;
	}
}

void SaveToFile::Impl::start()
{
	buffer.resize(channels);
	for(auto& channel: buffer) channel.clear();
	channelId = 0;
	displayResult->setText("Recording"s);
}

void SaveToFile::Impl::stop()
{
	std::size_t n = std::numeric_limits<std::size_t>::max();
	for (const auto& channel : buffer) n = std::min(n, channel.size());
	for (auto& channel : buffer) 
		channel.resize(n); // keep the same amount of samples on each channel
	if (buffer.size() && n) {
		AudioFile<double> file;
		file.setAudioBuffer(buffer);
		file.setNumChannels(channels);
		file.setNumSamplesPerChannel(n);
		file.setBitDepth(24);
		file.setSampleRate(sampleRate);
		if (file.save(dirName + '/' + fname + ".wav", AudioFileFormat::Wave)) {
			displayResult->setText("Saved"s);
		}
		else {
			displayResult->setText("Failed to save"s);
		}
	}
}