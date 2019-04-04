#include "guiElements.h"

#include <deque>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <variant>
#include <string>
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

	sf::View getCroppedView(const sf::View& oldView, SynthFloat x, SynthFloat y, SynthFloat w, SynthFloat h)
	{
		const auto center = SynthVec2{ std::round(x + w / 2), std::round(y + h / 2) };
		const auto size = SynthVec2{ w, h };
		sf::View ret{ sf::Vector2f(center), sf::Vector2f(size) };

		const auto& oldViewport = SynthRect{ oldView.getViewport() };
		auto oldSize = SynthVec2{ oldView.getSize() };
		oldSize.x = std::round(oldSize.x * 1.f / oldViewport.width);
		oldSize.y = std::round(oldSize.y * 1.f / oldViewport.height);
		const auto ratio = SynthVec2{ 
			size.x / oldSize.x, 
			size.y / oldSize.y 
		};
		const auto pos = SynthVec2{ x / oldSize.x, y / oldSize.y };

		ret.setViewport(sf::FloatRect(pos.x, pos.y, ratio.x, ratio.y));
		return ret;
	}

	sf::View getCroppedView(const sf::View& oldView, const SynthVec2& p, const SynthVec2& s)
	{
		return getCroppedView(oldView, p.x, p.y, s.x, s.y);
	}

	sf::View getCroppedView(const sf::View& oldView, const SynthRect& box)
	{
		return getCroppedView(oldView, box.left, box.top, box.width, box.height);
	}
}

const SynthVec2 SynthKey::whiteSizeDefault(100, 200);
const SynthVec2 SynthKey::blackSizeDefault(40, 150);
sf::Font TextDisplay::font = loadCourierNew();

SynthKey::SynthKey(Type t, SynthFloat px, SynthFloat py, const SynthVec2& size)
    :type(t)
{
    setPosition(px, py);
	setSize(sf::Vector2f(size));

    if (t == Type::Black) {
        if(size.x == 0) setSize(sf::Vector2f(SynthKey::blackSizeDefault));
        setFillColor(sf::Color::Black);
    }
    else {
		if (size.x == 0) setSize(sf::Vector2f(SynthKey::whiteSizeDefault));
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

void GuiElement::forwardEvent(const SynthEvent& event, const sf::Transform& transform)
{
	globalTransform = getTransform() * transform;
	onEvent(event);
	for (auto& child : children) child->forwardEvent(event, globalTransform);
}

void GuiElement::addChildren(const std::vector<std::shared_ptr<GuiElement>>& newChildren)
{
	children.insert(children.end(), newChildren.begin(), newChildren.end());
}

void GuiElement::removeChild(const std::shared_ptr<GuiElement>& child)
{
	auto found = std::find(children.begin(), children.end(), child);
	if (found != children.end()) {
		children.erase(found);
	}
}

void GuiElement::moveAroundPoint(const SynthVec2 & center)
{
	const auto& bounds = AABB();
	auto newPos = center - SynthVec2(bounds.width, bounds.height) / 2.;
	setPosition(std::round(newPos.x), std::round(newPos.y));
}

void GuiElement::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	drawImpl(target, states);
	for (auto& child : children) {
		child->draw(target, states);
	}
}

SynthRect GuiElement::AABB() const
{
	return SynthRect(0, 0, 0, 0);
}

SynthKeyboard::SynthKeyboard(SynthFloat px, SynthFloat py, callback_t eventCallback)
    : onKey(eventCallback)
{
	setPosition(px, py);

    const auto& w = SynthKey::White;
    const auto& b = SynthKey::Black;

    keys.reserve(24);
    std::basic_string<SynthKey::Type> orderedKeys = {w, b, w, b, w, w, b, w, b, w, b, w};
    orderedKeys = orderedKeys + orderedKeys + orderedKeys[0];
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
    for (const auto& key: keys)
        if (key.getType() == SynthKey::White)
            target.draw(key, states);

    for (const auto& key: keys)
        if (key.getType() == SynthKey::Black)
            target.draw(key, states);
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

SynthRect SynthKeyboard::AABB() const
{
	SynthVec2 size;
	size.x = keys.back().getPosition().x - keys.front().getPosition().x + keys.back().getSize().x;
	size.y = std::max(whiteSize.y, blackSize.y);
	return SynthRect{ SynthVec2(getPosition()), size };
}

void SynthKeyboard::repositionKeys()
{
    unsigned whitesCount = 0;
    SynthFloat dif = whiteSize.x - blackSize.x / 2;
    for (unsigned i=0; i<keys.size(); ++i) {
        auto& key = keys[i];
        if (key.getType() == SynthKey::White) {
            key.setPosition(whitesCount* key.getSize().x, 0);
            ++whitesCount;
        }
        else {
            key.setPosition(keys[i-1].getPosition().x + dif, 0);
        }
    }
}

void Slider::refreshText()
{
	std::ostringstream oss;
	oss << std::setprecision(2) << std::fixed << value;
	title.setText(name + "\n[" + oss.str() + "]");
	const auto& s = mainRect.getSize();
	const auto& p = SynthVec2(0, 0);
	title.moveAroundPoint({ p.x + s.x / 2.f, p.y - 20 });
}

Slider::Slider(const std::string& str, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::function<void()> callback)
	:title(str, 0, 0, 0, 0, titleSize), from(from), to(to), name(str), orientation(ori), value((from + to) / 2.), onMove(callback), size(sx, sy)
{
	const auto& minDim = std::min(sx, sy) / 2;
	sliderRectSize = SynthVec2(minDim, minDim);

	if (orientation == Vertical) {
		mainRect.setSize(sf::Vector2f(size));
	}
	else {
		mainRect.setSize(sf::Vector2f(size.y, size.x));
	}

	setPosition( px, py );

	mainRect.setPosition({ 0,0 });
    mainRect.setOutlineColor(sf::Color(0x757575FF));
    mainRect.setFillColor(sf::Color(0x660000FF));
    mainRect.setOutlineThickness(1.);

	sliderRect.setSize(sf::Vector2f(sliderRectSize));
    sliderRect.setPosition(mainRect.getSize().x/2 - sliderRectSize.x/2, mainRect.getSize().y/2 - sliderRectSize.y/2);
    sliderRect.setOutlineColor(sf::Color(0x757575FF));
    sliderRect.setFillColor(sf::Color(0x003366FF));
    sliderRect.setOutlineThickness(2.);

	refreshText();
}

Slider::Slider(const std::string& str, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::atomic<double>& val)
    : Slider(str, from, to, px, py, sx, sy, titleSize, ori, [&](){val=getValue();})
{
}

void Slider::onEvent(const SynthEvent& eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		const sf::Event& event = std::get<sf::Event>(eventArg);
		switch (event.type)
		{
			case sf::Event::MouseButtonPressed: {
				const auto& mousePos = SynthVec2(event.mouseButton.x, event.mouseButton.y);
				if (containsPoint(mousePos))
					clicked = true;
				break;
			}
			case sf::Event::MouseButtonReleased: {
				clicked = false;
				if (fixed) {
					auto[px, py] = mainRect.getPosition();
					sliderRect.setPosition(px + mainRect.getSize().x / 2 - sliderRectSize.x / 2, py + mainRect.getSize().y / 2 - sliderRectSize.y / 2);
					moveSlider(SynthVec2(px + mainRect.getSize().x / 2, py + mainRect.getSize().y / 2));
					refreshText();
					if (onMove) onMove();
				}
				break;
			}
			case sf::Event::MouseMoved: {
				if (clicked) {
					const auto& mousePos = globalTransform.getInverse() * sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
					moveSlider(SynthVec2(mousePos));
					refreshText();
					if (onMove) onMove();
				}
				break;
			}
			default: {
				break;
			}
		}
	}
}

SynthRect Slider::AABB() const
{
	const SynthRect 
		&box1{ title.AABB() }, 
		&box2{ SynthVec2(mainRect.getPosition()), SynthVec2(mainRect.getSize()) };
	return {
		std::min(box1.left, box2.left),
		std::min(box1.top, box2.top),
		std::max(box1.width, box2.width),
		std::max(box1.height, box2.height)
	};
}

void Slider::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(title, states);
    target.draw(mainRect, states);
    target.draw(sliderRect, states);
}

bool Slider::containsPoint(const SynthVec2& p) const
{
    const auto& rect = sliderRect;
    const auto& u = globalTransform * rect.getPosition();
    const auto& v = u + rect.getSize();

    return u.x <= p.x && p.x <= v.x &&
        u.y <= p.y && p.y <= v.y;
}

void Slider::moveSlider(const SynthVec2& p)
{
    const auto& minPos = mainRect.getPosition();
    const auto& maxPos = minPos + mainRect.getSize() - sliderRect.getSize();
    const auto& rectSize = maxPos - minPos;
    const auto& currentPos = sliderRect.getPosition();
    const auto& sliderSize = sliderRect.getSize();
    const auto& midPos = minPos + rectSize / 2.f;

    double newValueNormalized;

    if (orientation == Vertical) {
        sliderRect.setPosition(currentPos.x, std::clamp(p.y-sliderSize.y/2, SynthFloat(minPos.y), SynthFloat(maxPos.y)));
        newValueNormalized = (midPos.y - sliderRect.getPosition().y) / rectSize.y*2;
    }
    else {
        sliderRect.setPosition(std::clamp(p.x-sliderSize.x/2, SynthFloat(minPos.x), SynthFloat(maxPos.x)), currentPos.y);
        newValueNormalized = (sliderRect.getPosition().x - midPos.x) / rectSize.x*2;
    }

    value = from + (newValueNormalized+1) / 2. * (to-from);
}

Oscilloscope::Oscilloscope(SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned res, double speed)
    : resolution(res), speed(speed)
{
    setPosition(px, py);

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

SynthRect Oscilloscope::AABB() const
{
	return { SynthVec2(getPosition()), SynthVec2(window.getSize()) };
}

void Oscilloscope::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(window, states);
    target.draw(vArray.data(), resolution, sf::LineStrip, states);
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

const SynthVec2 TextDisplay::topLeftAlignment() const
{
	const auto& boundingBox = text.getGlobalBounds();
	const auto& topleft = sf::Vector2f( boundingBox.left, boundingBox.top );
	return SynthVec2(topleft - text.getPosition());
}

void TextDisplay::centralize()
{
	const auto& textBounds = text.getGlobalBounds();
	const auto& textDim    = sf::Vector2f( textBounds.width, textBounds.height );
	const auto& topLeftPos = sf::Vector2f( textBounds.left, textBounds.top );

	auto dif = (frame.getSize() - textDim) / 2.f - (topLeftPos - text.getPosition());

	dif.x = std::round(dif.x);
	dif.y = std::round(dif.y);

	text.setPosition(dif);
}

void TextDisplay::setFixedSize(bool fixed)
{
	fixedFrame = fixed;
}

void TextDisplay::setFrameSize(const SynthVec2 & size)
{
	if (fixedFrame) return;
	frame.setSize(sf::Vector2f(size));
}

void TextDisplay::fitFrame(const SynthVec2& size)
{
	if (fixedFrame) return;
	const auto& boundingBox = SynthRect(text.getGlobalBounds());
	frame.setSize(sf::Vector2f(
		std::max(size.x, boundingBox.width), 
		std::max(size.y, boundingBox.height) 
	));
}

TextDisplay::TextDisplay(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int size)
    : text()
{
    text.setFont(font);
    text.setCharacterSize(size);
    text.setString(initialText);
    text.setPosition(std::round(px), std::round(py));
    text.setFillColor(sf::Color::Green);

	frame.setOutlineThickness(0);
	frame.setFillColor(sf::Color::Transparent);
	frame.setPosition(0, 0);

	fitFrame({ sx, sy });

	const auto& boundingBox = text.getGlobalBounds();
	setPosition(boundingBox.left, boundingBox.top);

	text.setPosition(sf::Vector2f(-topLeftAlignment()));
}

std::unique_ptr<TextDisplay> TextDisplay::DefaultText(
	const std::string & initialText, 
	SynthFloat px, SynthFloat py, 
	unsigned int charSize)
{
	return std::make_unique<TextDisplay>(initialText, px, py, 0, 0, charSize);
}

std::unique_ptr<TextDisplay> TextDisplay::Multiline(
	const std::string initialText, 
	SynthFloat px, 
	SynthFloat py, 
	SynthFloat width, 
	unsigned int charSize
)
{
	sf::Text dummyText;
	dummyText.setFont(font);
	dummyText.setCharacterSize(charSize);
	dummyText.setPosition(px, py);

	std::string row, result;
	std::istringstream words(initialText);
	auto wordIt = std::istream_iterator<std::string>(words);
	result = *wordIt;
	row = *wordIt;
	unsigned wordCount = 1;
	while(++wordCount, ++wordIt != std::istream_iterator<std::string>()) {
		row.append(" "+*wordIt);
		dummyText.setString(row);
		const auto& bounds = dummyText.getGlobalBounds();
		if (wordCount > 1 && bounds.width > width) {
			result.append("\n");
			row = *wordIt;
			wordCount = 1;
		}
		else {
			result.append(" ");
		}
		result.append(*wordIt);
	}

	return TextDisplay::DefaultText(result, px, py, charSize);
}

void TextDisplay::setBgColor(const sf::Color& color)
{
	frame.setFillColor(color);
}

const sf::Color& TextDisplay::getBgColor() const 
{ 
	return frame.getFillColor();
}

SynthRect TextDisplay::AABB() const
{
	return { SynthVec2(getPosition()), SynthVec2(frame.getSize()) };
}

void TextDisplay::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(frame, states);
    target.draw(text, states);
}

void TextDisplay::setText(const std::string& content)
{
    text.setString(content);
	fitFrame();
}

Button::Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick)
    :TextDisplay(initialText, px, py, sx, sy, charSize), clickCallback(onClick)
{
	centralize();
	frame.setOutlineColor(sf::Color::White);
	frame.setOutlineThickness(1);
}


void Button::onEvent(const SynthEvent& eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		const sf::Event& event = std::get<sf::Event>(eventArg);
		static bool black = true;
		if (event.type == sf::Event::MouseButtonPressed) {
			SynthRect rect = { SynthVec2(globalTransform * frame.getPosition()), SynthVec2(frame.getSize()) };
			if (rect.contains(SynthVec2(event.mouseButton.x, event.mouseButton.y))) {
				black = !black;
				frame.setFillColor(black ? sf::Color::Black : sf::Color(0xaaaaaaaa));
				clickCallback();
			}
		}
	}
}

Window::Window(SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, const sf::Color & fillColor)
	:mainRect( sf::Vector2f(sx, sy) )
{
	setPosition(px, py);
	mainRect.setFillColor(fillColor);
	
	mainRect.setOutlineThickness(1);
	mainRect.setOutlineColor(sf::Color::Green);
}

void Window::drawImpl(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(mainRect, states);
}

void Window::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	const auto savedView = target.getView();
	target.setView(getCroppedView(savedView, SynthRect(states.transform.transformRect(sf::FloatRect(AABB())))));

	GuiElement::draw(target, states);

	target.setView(savedView);
}

void Window::setSize(const SynthVec2 & size)
{
	mainRect.setSize(sf::Vector2f(size));
}

SynthRect Window::AABB() const
{
	return { SynthVec2(getPosition()), SynthVec2(mainRect.getSize()) };
}
