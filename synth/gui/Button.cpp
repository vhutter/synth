#include "Button.h"

Button::Button(std::string initialText, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick)
	:TextDisplay(std::move(initialText), sx, sy, charSize), clickCallback(onClick)
{
	setFixedSize(true);
	setFocusable(true);
	centralize();
	frame.setOutlineColor(sf::Color::White);
	frame.setOutlineThickness(-1);
	refreshCol();

	setEventCallback([this](const sf::Event& event){
		lastEvent = event;
		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Button::Left) {
			if (isPressedOrReleased(event)) {
				pressed = true;
				refreshCol();
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Left) {
			if(pressed || passesAllClicks) clickCallback();
			pressed = false;
			refreshCol();
		}
	});
}

void Button::refreshCol()
{
	frame.setFillColor(pressed ? pressedCol : normalCol);
}

void Button::setNormalColor(const sf::Color & col)
{
	normalCol = col;
	refreshCol();
}

void Button::setPressedColor(const sf::Color & col)
{
	pressedCol = col;
	refreshCol();
}

bool Button::isPressed() const
{
	return pressed;
}

bool Button::needsEvent(const SynthEvent & event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return false;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased)
		return true;
	return false;
}

bool Button::isPressedOrReleased(const sf::Event& event) const
{
	sf::Vector2f mPos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
	return globalTransform.transformRect({ frame.getPosition(), sf::Vector2f(getSize()) }).contains(mPos);
}
