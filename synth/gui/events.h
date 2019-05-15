#ifndef SYNTHEVENT_H_DEFINED
#define SYNTHEVENT_H_DEFINED

#include <SFML/Window.hpp>
#include <RtMidi.h>

#include <queue>
#include <mutex>
#include <variant>
#include <optional>

class MidiEvent
{
public:
	enum class Type : uint8_t {
		KEYUP = 0b1000'0000,
		KEYDOWN = 0b1001'0000,
		KNOB = 0b1011'0000,
		WHEEL = 0b1110'0000,
	};
	using Key_t = uint8_t;
	using Velocity_t = uint8_t;
	using WheelValue_t = uint16_t;
	static constexpr WheelValue_t wheelValueMax() { return WheelValue_t{0b00111111'11111111}; }
	static constexpr WheelValue_t velocityMax()   { return Velocity_t  {0b01111111}; }
	static constexpr Key_t        keyMax()        { return Key_t       {0b01111111}; }

	MidiEvent(double t = 0., const std::vector<unsigned char>& msg = {});
	double getTime() const;
	const std::vector<unsigned char>& getRawMessage() const;
	const Type         getType() const;
	const Key_t        getKey() const;
	const Velocity_t   getVelocity() const;
	const WheelValue_t getWheelValue() const;
	const double       getVelocityNorm() const;
	const double       getWheelValueNorm() const;
	const double       getWheelKnobNorm() const;

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

using SynthEvent = std::variant<MidiEvent, sf::Event>;

#endif //SYNTHEVENT_H_DEFINED