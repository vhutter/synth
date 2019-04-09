#include "events.h"

#include <exception>

MidiEvent::MidiEvent(double t, const std::vector<unsigned char>& msg)
	:timestamp(t), message(msg)
{}

double MidiEvent::getTime() const 
{ 
	return timestamp; 
}

const std::vector<unsigned char>& MidiEvent::getMessage() const 
{ 
	return message; 
}

bool MidiContext::openPort(unsigned p)
{
	try {
		midiInput.openPort(p);
		midiInput.ignoreTypes(false, false, false);
		midiInput.setCallback([](double dt, std::vector<unsigned char>* msg, void* userData) {
			if (msg->size() > 0) {
				auto data = static_cast<void**>(userData);
				auto& q = *static_cast<decltype(msgQueue)*>(data[0]);
				auto& m = *static_cast<decltype(midiMutex)*>(data[1]);
				std::lock_guard<std::mutex> lock(m);
				q.emplace(dt, *msg);
			}
		}, userData);
		return true;
	}
	catch (...) {
		return false;
	}
}

MidiContext::~MidiContext()
{
	midiInput.closePort();
}

MidiContext::MidiContext(std::optional<unsigned> port)
{
	unsigned portCount = midiInput.getPortCount();
	if (portCount > 0){
		if (!port) {
			for (unsigned i = 0; i < portCount; i++) {
				if (openPort(i)) {
					// A default port is opened
					openedPort = i;
					return;
				}
			}
		}
		else {
			// The selected port is opened
			openedPort = port.value();
		}
	}
}

const std::optional<unsigned>& MidiContext::getPort() const
{
	return openedPort;
}

MidiContext::operator bool() const
{
	return openedPort.has_value();
}

bool MidiContext::pollEvent(MidiEvent& event)
{
	std::lock_guard<std::mutex> lock(midiMutex);
	if (msgQueue.empty()) {
		return false;
	}
	else {
		event = std::move(msgQueue.front());
		msgQueue.pop();
		return true;
	}
}


std::vector<std::string> MidiContext::deviceList()
{
	RtMidiIn midiInput;
	std::vector<std::string> ret;
	for (unsigned int i = 0; i < midiInput.getPortCount(); i++) {
		ret.emplace_back(midiInput.getPortName(i));
	}
	return ret;
}