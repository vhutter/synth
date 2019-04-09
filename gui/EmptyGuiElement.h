#ifndef EMPTYGUIELEMENT_H_INCLUDED
#define EMPTYGUIELEMENT_H_INCLUDED

#include "../guiElements.h"
#include "GuiElement.h"

class EmptyGuiElement : public GuiElement
{
	using sfmlCallback_t = std::function<void(const sf::Event&)>;
	using midiCallback_t = std::function<void(const MidiEvent&)>;

public:
	EmptyGuiElement(const sfmlCallback_t& sfml = {}, const midiCallback_t& midi = {});

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override {}
	virtual void onSfmlEvent(const sf::Event& event) { if (sfmlCallback) sfmlCallback(event); }
	virtual void onMidiEvent(const MidiEvent& event) { if (midiCallback) midiCallback(event); }

	sfmlCallback_t sfmlCallback;
	midiCallback_t midiCallback;
};

#endif //EMPTYGUIELEMENT_H_INCLUDED