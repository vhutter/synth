#include "Button.h"

Button::Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick)
	:TextDisplay(initialText, px, py, sx, sy, charSize), clickCallback(onClick)
{
	centralize();
	frame.setOutlineColor(sf::Color::White);
	frame.setOutlineThickness(-1);
}

void Button::setNormalColor(const sf::Color & col)
{
	normalCol = col;
}

void Button::setPressedColor(const sf::Color & col)
{
	pressedCol = col;
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


void Button::onSfmlEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed) {
		SynthRect rect = { SynthVec2(globalTransform * frame.getPosition()), SynthVec2(frame.getSize()) };
		if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
			pressed = !pressed;
			frame.setFillColor(pressed ? pressedCol : normalCol);
			clickCallback();
		}
	}
}