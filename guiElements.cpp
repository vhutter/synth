#include "guiElements.h"

#include <deque>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <variant>
#include <bitset>

namespace
{
    sf::Font loadCourierNew()
    {
        sf::Font tmpFont;
        if (!tmpFont.loadFromFile("fonts/cour.ttf")) {
            throw std::runtime_error("fonts/cour.ttf not found");
        }
        return tmpFont;
    }
}

const sf::Vector2f SynthKey::whiteSize(100, 200);
const sf::Vector2f SynthKey::blackSize(40, 150);
sf::Font TextDisplay::font = loadCourierNew();

SynthKey::SynthKey(Type t, float px, float py)
    :type(t)
{
    setPosition(sf::Vector2f(px, py));

    if (t == Type::Black) {
        setSize(SynthKey::blackSize);
        setFillColor(sf::Color::Black);
    }
    else {
        setSize(SynthKey::whiteSize);
        setFillColor(sf::Color::White);
    }

    setOutlineColor(sf::Color(0x4C0099FF));
    setOutlineThickness(4.);
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

bool SynthKey::isPressed() const
{
    return pressed;
}

void GuiElement::forwardEvent(const SynthEvent& event)
{
	onEvent(event);
	for (auto& child : children) child->forwardEvent(event);
}

void GuiElement::addChildren(std::vector<std::shared_ptr<GuiElement>> newChildren)
{
	children.insert(children.end(), newChildren.begin(), newChildren.end());
	for (auto child : children) {
		child->parent = std::enable_shared_from_this<GuiElement>::weak_from_this();
	}
}

void GuiElement::removeChild(std::shared_ptr<GuiElement> child)
{
	auto found = std::find(children.begin(), children.end(), child);
	if (found != children.end()) {
		child->parent.reset();
		children.erase(found);
	}
}

void GuiElement::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	drawImpl(target, states);
	for (auto& child : children) child->drawImpl(target, states);
}

SynthKeyboard::SynthKeyboard(float px, float py, callback_t eventCallback)
    : pos(px, py), onKey(eventCallback)
{
    const auto& w = SynthKey::White;
    const auto& b = SynthKey::Black;

    keys.reserve(24);
    std::basic_string<SynthKey::Type> orderedKeys = {w, b, w, b, w, w, b, w, b, w, b, w};
    orderedKeys = orderedKeys+orderedKeys+orderedKeys[0];
    for (auto type: orderedKeys)
        keys.push_back(SynthKey(type));

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
	lastPressed = false;
	if (value >= keys.size())
		return;

	eventType &= 0b1111'0000;
	switch (static_cast<MsgType>(eventType))
	{
	case MsgType::KEYDOWN:
		lastPressed = true;
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


	lastPressed = false;

	if (value == -1)
		return;

	switch (event.type)
	{
	case sf::Event::KeyPressed: {
		lastPressed = true;
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
    for (const auto& key: keys)
        if (key.getType() == SynthKey::White)
            target.draw(key);

    for (const auto& key: keys)
        if (key.getType() == SynthKey::Black)
            target.draw(key);
}

void SynthKeyboard::onEvent(const SynthEvent& eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		onSfmlEvent(std::get<sf::Event>(eventArg));
	}
	else {
		onMidiEvent(std::get<MidiEvent>(eventArg));
	}
}

void SynthKeyboard::setPosition(const sf::Vector2f& newPos)
{
    pos = newPos;
    repositionKeys();
}

void SynthKeyboard::repositionKeys()
{
    unsigned whitesCount = 0;
    float dif = SynthKey::whiteSize.x - SynthKey::blackSize.x / 2;
    for (unsigned i=0; i<keys.size(); ++i) {
        auto& key = keys[i];
        if (key.getType() == SynthKey::White) {
            key.setPosition(sf::Vector2f(pos.x+ whitesCount*SynthKey::whiteSize.x, pos.y));
            ++whitesCount;
        }
        else {
            key.setPosition(sf::Vector2f(keys[i-1].getPosition().x + dif, pos.y));
        }
    }
}


Slider::Slider(const std::string& str, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::function<void()> callback)
    :title(TextDisplay::AroundPoint(str, int(px+sx/2), int(py-40), 0,0,titleSize)), from(from), to(to), name(str), orientation(ori), value((from+to)/2.), onMove(callback), size(sx, sy)
{
    const auto& minDim = std::min(sx, sy)/2;
    sliderRectSize = sf::Vector2(minDim, minDim);

    if (orientation == Vertical) {
        mainRect.setSize(size);
    }
    else {
        mainRect.setSize(sf::Vector2f(size.y, size.x));
    }

    mainRect.setPosition(sf::Vector2f(px, py));
    mainRect.setOutlineColor(sf::Color(0x757575FF));
    mainRect.setFillColor(sf::Color(0x660000FF));
    mainRect.setOutlineThickness(2.);

    sliderRect.setSize(sliderRectSize);
    sliderRect.setPosition(sf::Vector2f(px + mainRect.getSize().x/2 - sliderRectSize.x/2, py + mainRect.getSize().y/2 - sliderRectSize.y/2));
    sliderRect.setOutlineColor(sf::Color(0x757575FF));
    sliderRect.setFillColor(sf::Color(0x003366FF));
    sliderRect.setOutlineThickness(2.);
}

Slider::Slider(const std::string& str, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::atomic<double>& val)
    : Slider(str, from, to, px+sx/2, py, sx, sy, titleSize, ori, [&](){val=getValue();})
{
}

void Slider::onEvent(const SynthEvent& eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		const sf::Event& event = std::get<sf::Event>(eventArg);
		switch (event.type)
		{
			case sf::Event::MouseButtonPressed: {
				const auto& mousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
				if (containsPoint(mousePos))
					clicked = true;
				break;
			}
			case sf::Event::MouseButtonReleased: {
				clicked = false;
				if (fixed) {
					auto[px, py] = mainRect.getPosition();
					sliderRect.setPosition(sf::Vector2f(px + mainRect.getSize().x / 2 - sliderRectSize.x / 2, py + mainRect.getSize().y / 2 - sliderRectSize.y / 2));
					moveSlider(sf::Vector2f(px + mainRect.getSize().x / 2, py + mainRect.getSize().y / 2));
					onMove();
				}
				break;
			}
			case sf::Event::MouseMoved: {
				if (clicked) {
					const auto& mousePos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
					moveSlider(mousePos);
					onMove();
				}
				break;
			}
			default: {
				break;
			}
		}
	}
}

void Slider::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
    std::ostringstream oss;
    oss << std::setprecision(2) << std::fixed << value;
    const_cast<TextDisplay*>(&title)->setText(name+"\n["+oss.str()+"]");
    target.draw(title, states);
    target.draw(mainRect, states);
    target.draw(sliderRect, states);
}

bool Slider::containsPoint(const sf::Vector2f& p) const
{
    const auto& rect = sliderRect;
    const auto& u = rect.getPosition();
    const auto& v = u + rect.getSize();

    return u.x <= p.x && p.x <= v.x &&
        u.y <= p.y && p.y <= v.y;
}

void Slider::moveSlider(const sf::Vector2f& p)
{
    const auto& minPos = mainRect.getPosition();
    const auto& maxPos = minPos + mainRect.getSize() - sliderRect.getSize();
    const auto& rectSize = maxPos - minPos;
    const auto& currentPos = sliderRect.getPosition();
    const auto& sliderSize = sliderRect.getSize();
    const auto& midPos = minPos + rectSize / 2.f;

    double newValueNormalized;

    if (orientation == Vertical) {
        sliderRect.setPosition(currentPos.x, std::clamp(p.y-sliderSize.y/2, minPos.y, maxPos.y));
        newValueNormalized = (midPos.y - sliderRect.getPosition().y) / rectSize.y*2;
    }
    else {
        sliderRect.setPosition(std::clamp(p.x-sliderSize.x/2, minPos.x, maxPos.x), currentPos.y);
        newValueNormalized = (sliderRect.getPosition().x - midPos.x) / rectSize.x*2;
    }

    value = from + (newValueNormalized+1) / 2. * (to-from);
}

Oscilloscope::Oscilloscope(float px, float py, float sx, float sy, unsigned res, double speed)
    : resolution(res), speed(speed)
{
    window.setPosition(sf::Vector2f(px, py));
    window.setSize(sf::Vector2f(sx, sy));
    window.setOutlineColor(sf::Color::White);
    window.setFillColor(sf::Color::Black);
    window.setOutlineThickness(2.);

    vArray.reserve(resolution*2); // bigger buffer size for faster incoming samples
    vArray.resize(resolution);
    const auto& pos = window.getPosition();
    const auto& size = window.getSize();
    for (unsigned i=0; i<resolution; ++i) {
        vArray[i].position.x = pos.x+i*size.x/resolution;
        vArray[i].position.y = pos.y+size.y/2;
        vArray[i].color = sf::Color::Green;
    }
}

void Oscilloscope::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(window, states);
    target.draw(vArray.data(), resolution, sf::LineStrip);
}


void Oscilloscope::newSamples(const std::vector<double>& samples) const
{
	const double halfY = window.getSize().y / 2;
    const unsigned dif = samples.size();
    if (dif < vArray.size()) {
        for (auto i=vArray.begin(); i<vArray.end()-dif; ++i)
            i->position.y = (i+dif)->position.y;
		const std::size_t shift = vArray.size() + dif;
        for (auto i=vArray.end()-dif; i<vArray.end(); ++i)
			i->position.y = window.getPosition().y + halfY + samples[i-vArray.begin()- shift] * halfY;
    }
    else {
        for (unsigned i=0; i<vArray.size(); ++i)
            vArray[i].position.y = window.getPosition().y + halfY + samples[i] * halfY;
    }
}

TextDisplay::TextDisplay(const std::string& initialText, float px, float py, float sx, float sy, unsigned int size)
    : text()
{
    text.setFont(font);
    text.setCharacterSize(size);
    text.setString(initialText);
    text.setPosition(sf::Vector2f(px, py));
    text.setFillColor(sf::Color::Green);

    const auto& boundingBox = text.getGlobalBounds();
    std::tie(sx, sy) = std::tie(std::max(sx, boundingBox.width), std::max(sy, boundingBox.height));
    std::tie(px, py) = std::tie(boundingBox.left, boundingBox.top);

    px = px - (sx-boundingBox.width)/2;
    py = py - (sy-boundingBox.height)/2;

    window.setPosition(sf::Vector2f(px, py));
    window.setSize(sf::Vector2f(sx, sy));
    window.setOutlineColor(sf::Color::Black);
    window.setFillColor(sf::Color::Black);
    window.setOutlineThickness(1.);
}

TextDisplay TextDisplay::AroundPoint(const std::string initialText, float px, float py, float sx, float sy, unsigned int charSize)
{
	TextDisplay ret(initialText, px, py, sx, sy, charSize);
    auto dif = ret.window.getSize() / 2.f;
	dif.x = floor(dif.x);
	dif.y = floor(dif.y);
    ret.window.setPosition(ret.window.getPosition()-dif);
    ret.text.setPosition  (ret.text.getPosition()-dif);
    return std::move(ret);
}

void TextDisplay::setBgColor(const sf::Color& color)
{
    window.setFillColor(color);
}

void TextDisplay::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(window, states);
    target.draw(text, states);
}

void TextDisplay::setText(const std::string& content)
{
    text.setString(content);
}

Button::Button(const std::string& initialText, float px, float py, float sx, float sy, unsigned int charSize, std::function<void()> onClick)
    :TextDisplay(initialText, px, py, sx, sy, charSize), clickCallback(onClick)
{
    window.setOutlineColor(sf::Color::White);
}


void Button::onEvent(const SynthEvent& eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		const sf::Event& event = std::get<sf::Event>(eventArg);
		static bool black = true;
		if (event.type == sf::Event::MouseButtonPressed && window.getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
			black = !black;
			window.setFillColor(black ? sf::Color::Black : sf::Color(0xaaaaaaaa));
			clickCallback();
		}
	}
}
