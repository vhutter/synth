#ifndef SYNTHEVENT_H_DEFINED
#define SYNTHEVENT_H_DEFINED

#include <SFML/Window.hpp>

#include "../RtMidi/RtMidi.h"
#include <queue>
#include <mutex>
#include <variant>
#include <optional>

class MidiEvent
{
public:
	MidiEvent(double t = 0., const std::vector<unsigned char>& msg = {});
	double getTime() const;
	const std::vector<unsigned char>& getMessage() const;

private:
	double timestamp;
	std::vector<unsigned char> message;
};

class MidiContext
{
public:
	MidiContext(std::optional<unsigned> port = {});
	MidiContext(const MidiContext& other) = delete;
	~MidiContext();

	const std::optional<unsigned>& getPort() const;
	operator bool() const;
	bool pollEvent(MidiEvent& event);

	static std::vector<std::string> deviceList();

private:
	bool openPort(unsigned p);

	RtMidiIn midiInput;
	std::optional<unsigned> openedPort;
	std::queue<MidiEvent> msgQueue;
	std::mutex midiMutex;
	void* userData[2] = { &msgQueue, &midiMutex };
};

typedef std::variant<MidiEvent, sf::Event> SynthEvent;

#endif //SYNTHEVENT_H_DEFINED