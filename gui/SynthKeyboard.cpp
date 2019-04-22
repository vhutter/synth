#include "SynthKeyboard.h"

#include <bitset>

const SynthVec2 SynthKey::whiteSizeDefault(100, 200);
const SynthVec2 SynthKey::blackSizeDefault(40, 150);

SynthKey::SynthKey(Type t, const SynthVec2& size)
	:type(t)
{
	setSize(sf::Vector2f(size));

	if (t == Type::Black) {
		if (size.x == 0) setSize(sf::Vector2f(SynthKey::blackSizeDefault));
		setFillColor(sf::Color::Black);
	}
	else {
		if (size.x == 0) setSize(sf::Vector2f(SynthKey::whiteSizeDefault));
		setFillColor(sf::Color::White);
	}

	setOutlineColor(sf::Color(0x4C0099FF));
	setOutlineThickness(1);
}

void SynthKey::setPressed(bool p)
{
	pressed = p;
	if (type == Type::White) {
		if (pressed) setFillColor(sf::Color(0xC0C0C0FF));
		else setFillColor(sf::Color::White);
	}
	else {
		if (pressed) setFillColor(sf::Color(0x404040FF));
		else setFillColor(sf::Color::Black);
	}
}

void SynthKeyboard::repositionKeys()
{
	unsigned whitesCount = 0;
	const auto& w = SynthKey::White;
	const auto& b = SynthKey::Black;
	unsigned n = octaveCount * 12 + 1;
	keys.clear();
	keys.reserve(n);
	std::basic_string<SynthKey::Type> orderedKeys = { w, b, w, b, w, w, b, w, b, w, b, w };
	for (unsigned i = 0; i < n; ++i) {
		auto type = orderedKeys[i % orderedKeys.size()];
		keys.push_back(SynthKey(type));
	}

	SynthFloat dif = whiteSize.x - blackSize.x / 2;
	for (unsigned i = 0; i < keys.size(); ++i) {
		auto& key = keys[i];
		if (key.type == SynthKey::White) {
			key.setPosition(whitesCount * whiteSize.x, 0);
			key.setSize(sf::Vector2f(whiteSize));
			++whitesCount;
		}
		else {
			key.setPosition(keys[i - 1].getPosition().x + dif, 0);
			key.setSize(sf::Vector2f(blackSize));
		}
	}
}

SynthKeyboard::SynthKeyboard(callback_t eventCallback)
	: onKey(eventCallback)
{
	repositionKeys();
}

void SynthKeyboard::setSize(SynthKey::Type type, const SynthVec2& size)
{
	if (type == SynthKey::White) {
		whiteSize = size;
	}
	else {
		blackSize = size;
	}
	repositionKeys();
}

void SynthKeyboard::setOctaveCount(unsigned count)
{
	octaveCount = count;
	repositionKeys();
}

void SynthKeyboard::onMidiEvent(const MidiEvent & event)
{
	unsigned char eventType = event.getMessage()[0];
	unsigned char keyCode = event.getMessage()[1];
	unsigned char intensity = event.getMessage()[2];

	static int msgId = 0;
	std::cout << msgId++ << event.getMessage().size() << ": " <<
		std::bitset<8>(eventType) << " " <<
		std::bitset<8>(keyCode) << " " <<
		std::bitset<8>(intensity) << "\n";

	enum class MsgType : uint8_t {
		KEYDOWN = 0b1001'0000,
		KEYUP = 0b1000'0000,
		KNOB = 0b1011'0000,
		WHEEL = 0b1110'0000,
	};

	unsigned char value = keyCode - 48;
	if (value >= keys.size())
		return;

	eventType &= 0b1111'0000;
	switch (static_cast<MsgType>(eventType))
	{
	case MsgType::KEYDOWN:
		onKey(value, SynthKey::State::Pressed);
		break;
	case MsgType::KEYUP:
		onKey(value, SynthKey::State::Released);
		break;
	default:
		break;
	}
}

void SynthKeyboard::onSfmlEvent(const sf::Event & event)
{
	const auto& key = event.key.code;

	int value = -1;
	switch (key)
	{
	case sf::Keyboard::Z:
		value = 0;
		break;
	case sf::Keyboard::S:
		value = 1;
		break;
	case sf::Keyboard::X:
		value = 2;
		break;
	case sf::Keyboard::D:
		value = 3;
		break;
	case sf::Keyboard::C:
		value = 4;
		break;
	case sf::Keyboard::V:
		value = 5;
		break;
	case sf::Keyboard::G:
		value = 6;
		break;
	case sf::Keyboard::B:
		value = 7;
		break;
	case sf::Keyboard::H:
		value = 8;
		break;
	case sf::Keyboard::N:
		value = 9;
		break;
	case sf::Keyboard::J:
		value = 10;
		break;
	case sf::Keyboard::M:
		value = 11;
		break;
	default:
		break;
	}

	if (value == -1)
		return;

	switch (event.type)
	{
	case sf::Event::KeyPressed: {
		onKey(value, SynthKey::State::Pressed);
		break;
	}
	case sf::Event::KeyReleased: {
		onKey(value, SynthKey::State::Released);
		break;
	}
	default:
		break;
	}
}

void SynthKeyboard::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& key : keys)
		if (key.type == SynthKey::White)
			target.draw(key, states);

	for (const auto& key : keys)
		if (key.type == SynthKey::Black)
			target.draw(key, states);
}

SynthRect SynthKeyboard::AABB() const
{
	SynthVec2 size;
	size.x = SynthFloat(keys.back().getPosition().x) - keys.front().getPosition().x + keys.back().getSize().x;
	size.y = std::max(whiteSize.y, blackSize.y);
	return SynthRect{ SynthVec2(getPosition()), size };
}

bool SynthKeyboard::needsEvent(const SynthEvent & event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return true;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::KeyPressed ||
		sfEvent.type == sf::Event::KeyReleased)
		return true;
	return false;
}

KeyboardOutput::KeyboardOutput()
{
	kb = std::make_unique<SynthKeyboard>([this](unsigned keyIdx, SynthKey::State keyState) {
		if (keyState == SynthKey::State::Pressed) {
			(*kb)[keyIdx].setPressed(true);
		}
		else {
			(*kb)[keyIdx].setPressed(false);
		}
		for (auto callback : callbacks) {
			callback(keyIdx, keyState);
		}
	});
	kb->setSize(SynthKey::Black, {SynthKey::blackSizeDefault/SynthFloat(2)});
	kb->setSize(SynthKey::White, {SynthKey::whiteSizeDefault/SynthFloat(2)});
	kb->setOctaveCount(4);
}

std::shared_ptr<SynthKeyboard> KeyboardOutput::getGuiElement() const
{
	return kb;
};

