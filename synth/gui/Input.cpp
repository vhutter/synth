#include "Input.h"

void InputField::activate()
{
	active = true;
	firstInput = true;
	setOutlineThickness(-1);
}

void InputField::deactivate()
{
	active = false;
	setOutlineThickness(0);
	if (onEndCallback) onEndCallback();
}

InputField::InputField(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize)
	:Button("", sx, sy, charSize, [this]() {
		if (isPressed()) {
			if(!active) activate();
		}
		else {
			if(active) deactivate();
		}
	}),
	eventHandler(std::make_unique<EmptyGuiElement>([this, type](const sf::Event& event) {
		if (event.type == sf::Event::TextEntered) {
			if (active) {
				auto code = event.text.unicode;
				auto oldText = getText();
				if (code == '\b') { //backspace
					setTextCentered(oldText.substring(0, oldText.getSize() - 1));
				}
				else if (code == '\t') { // tab
					// ignore
				}
				else if (code == '\r' || code == '\n' || code == 27) { //enter || escape
					deactivate();
				}
				else if (code < 128 && ( // ASCII
					(type & Int  ) && '0' <= code && code <= '9' ||
					(type & Point) && '.' == code                ||
					(type & Alpha) && 'a' <= code && code <= 'z' ||
					(type & Alpha) && 'A' <= code && code <= 'Z'
				)) {
					if (firstInput) {
						oldText = "";
						firstInput = false;
					}
					std::string newText = oldText + sf::String(event.text.unicode);

					setTextCentered(newText);
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
	setNormalColor(sf::Color(getConfig("inputFieldNormalColor")));
	setPressedColor(sf::Color(getConfig("inputFieldPressedColor")));
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
	return (
		sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased ||
		sfEvent.type == sf::Event::TextEntered ||
		sfEvent.type == sf::Event::KeyPressed
	);
	return false;
}

InputRecord::InputRecord(Type type, SynthFloat sx, SynthFloat sy, unsigned int charSize)
	:InputField(InputField::None, sx, sy, charSize),
	type(type)
{
	eventHandler->setCallback(EmptyGuiElement::sfmlCallback_t{});
	eventHandler->setCallback(EmptyGuiElement::midiCallback_t{});

	if (type & Sfml) {
		eventHandler->setCallback([this, type](const sf::Event & event) {
			if (active) {
				lastEvent = event;
				switch (event.type)
				{
					case sf::Event::KeyPressed:
						if (type & KeyboardButton) {
							setTextCentered("Keyboard: " + std::to_string(event.key.code));
							deactivate();
						}
						break;
					case sf::Event::MouseButtonPressed:
						if (type & MouseButton) {
							if (event.mouseButton.button != sf::Mouse::Button::Left || !isPressed()) {
								setTextCentered("Mouse: " + std::to_string(event.mouseButton.button));
								deactivate();
							}
						}
						break;
					case sf::Event::MouseWheelScrolled:
						if (type & MouseWheel) {
							setTextCentered("MouseWheel");
							deactivate();
						}
						break;
					default:
						;
				}
			}
			});
	}
	if (type & Midi) { // anything except T::KeyboardButton
		eventHandler->setCallback([this, type](const MidiEvent & event) {
			if (active) {
				lastEvent = event;
				switch (event.getType())
				{
					case MidiEvent::Type::KEYDOWN:
						if (type & MidiKey) {
							setTextCentered("Key: " + std::to_string(event.getKey()));
							deactivate();
						}
						break;
					case MidiEvent::Type::KNOB:
						if (type & MidiKnob) {
							setTextCentered("Knob: " + std::to_string(event.getKey()));
							deactivate();
						}
						break;
					case MidiEvent::Type::WHEEL:
						if (type & MidiWheel) {
							setTextCentered("Wheel");
							deactivate();
						}
						break;
					default:
						;
				}
			}
		});
	}
}

const SynthEvent& InputRecord::getLastEvent() const
{
	return lastEvent;
}

bool InputRecord::needsEvent(const SynthEvent& event) const
{
	if (!active)
		return InputField::needsEvent(event);
	if (std::holds_alternative<MidiEvent>(event)) {
		const auto& e = std::get<MidiEvent>(event);
		return (
			((type & MidiKnob )  && e.getType() == MidiEvent::Type::KNOB)    ||
			((type & MidiKey  )  && e.getType() == MidiEvent::Type::KEYDOWN) ||
			((type & MidiWheel)  && e.getType() == MidiEvent::Type::WHEEL)
		);
	}
	else {
		const auto& e = std::get<sf::Event>(event);
		return (
			e.type == sf::Event::MouseButtonReleased || // Deactivation click
			e.type == sf::Event::MouseButtonPressed  || //
			((type & MouseWheel    ) && e.type == sf::Event::MouseWheelScrolled)  ||
			((type & KeyboardButton) && e.type == sf::Event::KeyPressed)
		);
	}
	return false;
}