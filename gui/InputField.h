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

private:
	void activate();
	void deactivate();

	bool active{ false };
	std::function<void()> onEndCallback;

};

#endif //INPUTFIELD_H_INCLUDED