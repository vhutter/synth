#ifndef GUIELEMENT_H_INCLUDED
#define GUIELEMENT_H_INCLUDED

#include <functional>
#include <sfml/Graphics.hpp>
#include "../core/utility.h"
#include "events.h"

class GuiElement : public sf::Drawable, public sf::Transformable, public std::enable_shared_from_this<GuiElement>
{
public:
	virtual ~GuiElement() = default;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override final;
	virtual SynthRect AABB() const;
	virtual bool needsEvent(const SynthEvent& event) const { return true; }
	virtual bool forwardsEvent(const SynthEvent& event) const { return true; }
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const { return target.getView(); }
	void moveAroundPoint(const SynthVec2& center);
	bool forwardEvent(const SynthEvent& event, const sf::Transform& transform = {});
	void addChild(std::shared_ptr<GuiElement> child, int px=0, int py=0);
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
public:
	using midiCallback_t = std::function<void(const MidiEvent&)>;
	using sfmlCallback_t = std::function<void(const sf::Event&)>;
	using synthCallback_t = std::function<void(const SynthEvent&)>;

	EmptyGuiElement() = default;
	EmptyGuiElement(const sfmlCallback_t& sfml, const midiCallback_t& midi);
	EmptyGuiElement(const sfmlCallback_t& sfml);
	EmptyGuiElement(const midiCallback_t& midi);

	void setCallback(const sfmlCallback_t& sfml);
	void setCallback(const midiCallback_t& midi);

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override {}
	virtual void onSfmlEvent(const sf::Event& event) override { if (sfmlCallback) sfmlCallback(event); }
	virtual void onMidiEvent(const MidiEvent& event) override { if (midiCallback) midiCallback(event); }

	sfmlCallback_t sfmlCallback;
	midiCallback_t midiCallback;
};


#endif //GUIELEMENT_H_INCLUDED