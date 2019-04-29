#include "InputField.h"

void InputField::activate()
{
	active = true;
	setOutlineThickness(-1);
}

void InputField::deactivate()
{
	active = false;
	setOutlineThickness(0);
	if (onEndCallback) onEndCallback();
}

InputField::InputField(SynthFloat sx, SynthFloat sy, unsigned int charSize)
	:Button("", sx, sy, charSize, [this]() {
		if (isPressed()) {
			activate();
		}
		else {
			deactivate();
		}
	}),
	eventHandler(std::make_unique<EmptyGuiElement>([this](const sf::Event& event) {
		if (event.type == sf::Event::TextEntered) {
			if (active) {
				auto code = event.text.unicode;
				auto oldText = getText();
				if (code == 8) { //backspace
					setTextCentered(oldText.substring(0, oldText.getSize() - 1));
				}
				if (code == 10 || code == 13) { //enter
					deactivate();
				}
				else if (code < 128) { // ASCII
					setTextCentered(oldText + sf::String(event.text.unicode));
					const auto & aabb = text.getLocalBounds();
					if (aabb.width >= getSize().x) {
						setTextCentered(oldText);
					}
				}
			}
		}
	}))
{
	addChild(eventHandler);
	passesAllClicks = true;
	setOutlineColor(sf::Color::White);
	setNormalColor(sf::Color(0x330017ff));
	setPressedColor(sf::Color(0x330017ff));
	setCropping(false);	
	setOutlineThickness(0);
}

void InputField::setTextCentered(const std::string& str)
{
	setText(str);
	centralize();
}

void InputField::setOnEnd(std::function<void()> callback)
{
	onEndCallback = callback;
}

bool InputField::needsEvent(const SynthEvent& event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return false;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased ||
		sfEvent.type == sf::Event::TextEntered)
		return true;
	return false;
}

InputRecord::InputRecord(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize)
	:InputField(sx, sy, charSize)
{
	using T = InputRecord::Type;
	if (type == T::KeyboardButton) {
		eventHandler->setSfmlCallback([this, type](const sf::Event & event) {
			if (active) {
				if (event.type == sf::Event::KeyPressed) {
					setTextCentered("KeyCode: " + std::to_string(event.key.code));
					deactivate();
				}
				lastEvent = event;
			}
		});
	}
	else {
		eventHandler->setSfmlCallback([](const sf::Event&) {});
		eventHandler->setMidiCallback([this, type](const MidiEvent & event) {
			if (active) {
				if (type == T::MidiKey && event.getType() == MidiEvent::Type::KEYDOWN) {
					setTextCentered("KeyCode: " + std::to_string(event.getKey()));
					deactivate();
				}
				else if (type == T::MidiWheelKnob) {
					if (event.getType() == MidiEvent::Type::WHEEL) {
						setTextCentered("Wheel");
						deactivate();
					}
					else if (event.getType() == MidiEvent::Type::KNOB) {
						setTextCentered("Knob: " + std::to_string(event.getKey()));
						deactivate();
					}
				}
				else if (type == T::MidiKey && event.getType() == MidiEvent::Type::KEYDOWN) {
					setTextCentered("Key: " + std::to_string(event.getKey()));
					deactivate();
				}
				lastEvent = event;
			}
		});
	}
}

bool InputRecord::needsEvent(const SynthEvent& event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return true;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased ||
		sfEvent.type == sf::Event::KeyPressed)
		return true;
	return false;
}