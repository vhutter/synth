#ifndef INPUTFIELD_H_INCLUDED
#define INPUTFIELD_H_INCLUDED

#include "Button.h"
#include "../core/utility.h"

class InputField : public Button
{
public:
	using Type = uint8_t;
	enum : Type {
		None      = 0x00,
		Alpha     = 0b0001,
		Int       = 0b0010,
		Point     = 0b0100,
		Double    = Int | Point,
		AlphaNum  = Int | Alpha,
		Any       = 0xff,
	};

	InputField(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize = getConfig("defaultCharSize"));
	void setTextCentered(const std::string& str);
	void setOnEnd(std::function<void()> callback);

	virtual bool needsEvent(const SynthEvent& event) const override;

protected:
	void activate();
	void deactivate();

	std::shared_ptr<EmptyGuiElement> eventHandler;
	bool active{ false }, firstInput{ false };

private:
	std::function<void()> onEndCallback;
};

class InputRecord : public InputField
{
public:
	using Type = uint8_t;
	enum : Type {
		KeyboardButton = 0b0000'0001,
		MouseWheel     = 0b0000'0010,
		MouseButton    = 0b0000'0100,
		Mouse          = MouseWheel | MouseButton,
		Sfml           = KeyboardButton | Mouse,

		MidiKnob       = 0b0000'1000,
		MidiKey        = 0b0001'0000,
		MidiWheel      = 0b0010'0000,
		Midi           = MidiKnob | MidiKey | MidiWheel,

		Any            = Sfml | Midi,
	};

	InputRecord(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize = getConfig("defaultCharSize"));
	const SynthEvent& getLastEvent() const;

	virtual bool needsEvent(const SynthEvent& event) const override;

private:
	SynthEvent lastEvent;
	const Type type;
};

#endif //INPUTFIELD_H_INCLUDED