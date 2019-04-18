#include "Button.h"

Button::Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick)
	:TextDisplay(initialText, px, py, sx, sy, charSize), clickCallback(onClick)
{
	setFocusable(true);
	centralize();
	frame.setOutlineColor(sf::Color::White);
	frame.setOutlineThickness(-1);
	refreshCol();

	setEventCallback([this](const sf::Event& event){
		if (event.type == sf::Event::MouseButtonPressed) {
			SynthRect rect = { SynthVec2(globalTransform * frame.getPosition()), SynthVec2(frame.getSize()) };
			if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
				pressed = true;
				refreshCol();
				clickCallback();
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased) {
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

bool Button::needsEvent(const SynthEvent & event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return false;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased)
		return true;
	return false;
}