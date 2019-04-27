#ifndef GUIELEMENT_H_INCLUDED
#define GUIELEMENT_H_INCLUDED

#include <sfml/Graphics.hpp>
#include "../utility.h"
#include "events.h"

class GuiElement : public sf::Drawable, public sf::Transformable, public std::enable_shared_from_this<GuiElement>
{
public:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override final;
	virtual SynthRect AABB() const;
	virtual bool needsEvent(const SynthEvent& event) const { return true; }
	virtual bool forwardsEvent(const SynthEvent& event) const { return true; }
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const { return target.getView(); }
	void moveAroundPoint(const SynthVec2& center);
	bool forwardEvent(const SynthEvent& event, const sf::Transform& transform = {});
	void addChild(std::shared_ptr<GuiElement> child, unsigned px=0, unsigned py=0);
	void removeChild(const std::shared_ptr<GuiElement>& child);
	void onEvent(const SynthEvent& event);
	void setVisibility(bool v);
	void setFocusable(bool d);
	void focus(unsigned ownIdx=-1u);

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	virtual void onSfmlEvent(const sf::Event& event) {}
	virtual void onMidiEvent(const MidiEvent& event) {}

	std::vector<std::shared_ptr<GuiElement>> children;
	sf::Transform globalTransform; // Used for event handling
	bool visible{ true }, // visible elements are drawed and they forward events recursively
		focusable{ true };

private:
	using sf::Transformable::rotate;
	using sf::Transformable::scale;
	using sf::Transformable::setRotation;
	using sf::Transformable::setScale;

	GuiElement* parent{ nullptr };
};

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


#endif //GUIELEMENT_H_INCLUDED