#include "SynthKeyboard.h"

#include <optional>
#include "../utility.h"

SynthVec2 SynthKey::whiteSizeDefault()
{
	return { SynthFloat(getConfig("whiteKeyWidth")), SynthFloat(getConfig("whiteKeyHeight")) };
}
SynthVec2 SynthKey::blackSizeDefault()
{
	return { SynthFloat(getConfig("blackKeyWidth")), SynthFloat(getConfig("blackKeyHeight")) };
}

SynthKey::SynthKey(Type t, const SynthVec2& size)
	:type(t)
{
	setSize(sf::Vector2f(size));

	if (t == Type::Black) {
		if (size.x == 0) setSize(sf::Vector2f(SynthKey::blackSizeDefault()));
		setFillColor(sf::Color::Black);
	}
	else {
		if (size.x == 0) setSize(sf::Vector2f(SynthKey::whiteSizeDefault()));
		setFillColor(sf::Color::White);
	}

	setOutlineColor(sf::Color(getConfig("keyOutlineColor")));
	setOutlineThickness(1);
}

void SynthKey::setPressed(bool p)
{
	pressed = p;
	if (type == Type::White) {
		if (pressed) setFillColor(sf::Color(getConfig("whitePressedColor")));
		else setFillColor(sf::Color::White);
	}
	else {
		if (pressed) setFillColor(sf::Color(getConfig("blackPressedColor")));
		else setFillColor(sf::Color::Black);
	}
}

void SynthKeyboard::repositionKeys(unsigned keyCount)
{
	unsigned whitesCount = 0;
	const auto& w = SynthKey::White;
	const auto& b = SynthKey::Black;
	unsigned n = keyCount;
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

SynthKeyboard::SynthKeyboard(unsigned keyCount, callback_t eventCallback)
	: onKey([this, eventCallback](unsigned keyIdx, SynthKey::State keyState) {
		if (keyIdx < keys.size()) {
			eventCallback(keyIdx, keyState);
		}
	})
{
	repositionKeys(keyCount);
}

void SynthKeyboard::setSize(SynthKey::Type type, const SynthVec2& size)
{
	if (type == SynthKey::White) {
		whiteSize = size;
	}
	else {
		blackSize = size;
	}
	repositionKeys(keys.size());
}

void SynthKeyboard::onMidiEvent(const MidiEvent & event)
{
	unsigned char eventType = event.getRawMessage()[0];
	unsigned char keyCode = event.getRawMessage()[1];
	unsigned char intensity = event.getRawMessage()[2];

	const auto& message = event.getRawMessage();
	uint8_t least = message[1] & 0b0111'1111;
	uint8_t most = message[2] & 0b0111'1111;
	uint16_t all = least + (most << 7);

	unsigned char middleC = 48;
	unsigned char value = keyCode - middleC;
	if (value >= keys.size())
		return;

	switch (event.getType())
	{
	case MidiEvent::Type::KEYDOWN:
		onKey(value, SynthKey::State::Pressed);
		break;
	case MidiEvent::Type::KEYUP:
		onKey(value, SynthKey::State::Released);
		break;
	default:
		break;
	}
}

void SynthKeyboard::onSfmlEvent(const sf::Event & event)
{
	const auto& key = event.key.code;

	std::optional<unsigned> keyIdx;
	switch (key)
	{
	case sf::Keyboard::Z:
		keyIdx = 0;
		break;
	case sf::Keyboard::S:
		keyIdx = 1;
		break;
	case sf::Keyboard::X:
		keyIdx = 2;
		break;
	case sf::Keyboard::D:
		keyIdx = 3;
		break;
	case sf::Keyboard::C:
		keyIdx = 4;
		break;
	case sf::Keyboard::V:
		keyIdx = 5;
		break;
	case sf::Keyboard::G:
		keyIdx = 6;
		break;
	case sf::Keyboard::B:
		keyIdx = 7;
		break;
	case sf::Keyboard::H:
		keyIdx = 8;
		break;
	case sf::Keyboard::N:
		keyIdx = 9;
		break;
	case sf::Keyboard::J:
		keyIdx = 10;
		break;
	case sf::Keyboard::M:
		keyIdx = 11;
		break;
	default:
		break;
	}

	if (!keyIdx)
		return;

	keyIdx = 12 * octaveShift + keyIdx.value();

	switch (event.type)
	{
	case sf::Event::KeyPressed: {
		onKey(keyIdx.value(), SynthKey::State::Pressed);
		break;
	}
	case sf::Event::KeyReleased: {
		onKey(keyIdx.value(), SynthKey::State::Released);
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

SynthKey& SynthKeyboard::operator[](std::size_t i)
{
	return keys.at(i);
}

void SynthKeyboard::setOctaveShift(unsigned n)
{
	if (n * 12u < keys.size())
		octaveShift = n;
}

unsigned SynthKeyboard::getOctaveShift()
{
	return octaveShift;
}

KeyboardOutput::KeyboardOutput(unsigned keyCount)
	:kb(std::make_unique<SynthKeyboard>(keyCount, [this](unsigned keyIdx, SynthKey::State keyState) {
		if (keyState == SynthKey::State::Pressed) {
			(*kb)[keyIdx].setPressed(true);
		}
		else {
			(*kb)[keyIdx].setPressed(false);
		}
		for (auto callback : callbacks) {
			callback(keyIdx, keyState);
		}
	}))
{
	kb->setSize(SynthKey::Black, {SynthKey::blackSizeDefault()/SynthFloat(4)});
	kb->setSize(SynthKey::White, {SynthKey::whiteSizeDefault()/SynthFloat(4)});
}

std::shared_ptr<SynthKeyboard> KeyboardOutput::getSynthKeyboard() const
{
	return kb;
}

void KeyboardOutput::stopAll()
{
	for (unsigned i = 0; i < kb->keys.size(); ++i) {
		(*kb)[i].setPressed(false);
		for (auto callback : callbacks) {
			callback(i, SynthKey::State::Released);
		}
	}
}


