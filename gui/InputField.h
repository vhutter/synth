#ifndef INPUTFIELD_H_INCLUDED
#define INPUTFIELD_H_INCLUDED

#include "Button.h"

class InputField : public Button
{
public:
	InputField(SynthFloat sx, SynthFloat sy, unsigned int charSize=16);
	void setTextCentered(const std::string& str);
	void setOnEnd(std::function<void()> callback);

	virtual bool needsEvent(const SynthEvent& event) const override;

protected:
	void activate();
	void deactivate();

	std::shared_ptr<EmptyGuiElement> eventHandler;
	bool active{ false };

private:
	std::function<void()> onEndCallback;
};

class InputRecord : public InputField
{
public:
	enum Type : int {
		KeyboardButton = 0,
		MidiKey        = 4,
		MidiWheelKnob  = 5,
	};

	InputRecord(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize = 16);

	virtual bool needsEvent(const SynthEvent& event) const override;

private:
	SynthEvent lastEvent;

};

#endif //INPUTFIELD_H_INCLUDED