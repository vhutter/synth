#include "guiElements.h"

#include <deque>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <variant>
#include <string>
#include <bitset>
#include <limits>


//const SynthVec2 SynthKey::whiteSizeDefault(100, 200);
//const SynthVec2 SynthKey::blackSizeDefault(40, 150);
//
//SynthKey::SynthKey(Type t, SynthFloat px, SynthFloat py, const SynthVec2& size)
//    :type(t)
//{
//    setPosition(px, py);
//	setSize(sf::Vector2f(size));
//
//    if (t == Type::Black) {
//        if(size.x == 0) setSize(sf::Vector2f(SynthKey::blackSizeDefault));
//        setFillColor(sf::Color::Black);
//    }
//    else {
//		if (size.x == 0) setSize(sf::Vector2f(SynthKey::whiteSizeDefault));
//        setFillColor(sf::Color::White);
//    }
//
//    setOutlineColor(sf::Color(0x4C0099FF));
//    setOutlineThickness(4.);
//}
//
//// The return value indicates if the mouse event was used
//bool GuiElement::forwardEvent(const SynthEvent& event, const sf::Transform& transform)
//{
//	if (!visible)
//		return false;
//	bool ret = false;
//	if (auto e = std::get_if<sf::Event>(&event)) {
//		if (e->type == sf::Event::MouseButtonPressed && !dynamic_cast<EmptyGuiElement*>(this)) {
//			const auto& aabb = transform.transformRect(sf::FloatRect(AABB()));
//			if (!aabb.contains(sf::Vector2f(e->mouseButton.x, e->mouseButton.y))) {
//				return false;
//			}
//			else ret = true;
//		}
//	}
//	globalTransform = getTransform() * transform;
//	if (needsEvent(event)) {
//		if (forwardsEvent(event)) {
//			auto clickedChild = children.end();
//			for (auto child = children.rbegin(); child != children.rend(); ++child) {
//				if ((*child)->forwardEvent(event, globalTransform)) {
//					clickedChild = child.base();
//					std::advance(clickedChild, -1);
//					break;
//				}
//			}
//			if (clickedChild != children.end() && (*clickedChild)->dynamic) {
//				// set the element in focus
//				auto pElem = *clickedChild;
//				children.erase(clickedChild);
//				children.push_back(pElem);
//			}
//		}
//		onEvent(event);
//	}
//	return ret;
//}
//
//void GuiElement::addChildren(const std::vector<std::shared_ptr<GuiElement>>& newChildren)
//{
//	children.insert(children.end(), newChildren.begin(), newChildren.end());
//}
//
//void GuiElement::removeChild(const std::shared_ptr<GuiElement>& child)
//{
//	auto found = std::find(children.begin(), children.end(), child);
//	if (found != children.end()) {
//		children.erase(found);
//	}
//}
//
//void GuiElement::onEvent(const SynthEvent & eventArg)
//{
//	if (std::holds_alternative<sf::Event>(eventArg)) {
//		onSfmlEvent(std::get<sf::Event>(eventArg));
//	}
//	else {
//		onMidiEvent(std::get<MidiEvent>(eventArg));
//	}
//}
//
//void GuiElement::setVisibility(bool v)
//{
//	visible = v;
//}
//
//void GuiElement::setDynamic(bool d)
//{
//	dynamic = d;
//}
//
//void GuiElement::moveAroundPoint(const SynthVec2 & center)
//{
//	const auto& bounds = AABB();
//	auto newPos = center - SynthVec2(bounds.width, bounds.height) / 2.;
//	setPosition(std::round(newPos.x), std::round(newPos.y));
//}
//
//void GuiElement::draw(sf::RenderTarget& target, sf::RenderStates states) const
//{
//	if (visible) {
//		states.transform *= getTransform();
//		drawImpl(target, states);
//		const auto savedView = target.getView();
//		target.setView(childrenView(target, states));
//		for (auto& child : children) {
//			child->draw(target, states);
//		}
//		target.setView(savedView);
//	}
//}
//
//EmptyGuiElement::EmptyGuiElement(const sfmlCallback_t& sfml, const midiCallback_t& midi)
//	:sfmlCallback(sfml),
//	midiCallback(midi)
//{}
//
//SynthRect GuiElement::AABB() const
//{
//	return SynthRect();
//}
//
//SynthKeyboard::SynthKeyboard(SynthFloat px, SynthFloat py, callback_t eventCallback)
//    : onKey(eventCallback)
//{
//	setPosition(px, py);
//
//    const auto& w = SynthKey::White;
//    const auto& b = SynthKey::Black;
//
//    keys.reserve(24);
//    std::basic_string<SynthKey::Type> orderedKeys = {w, b, w, b, w, w, b, w, b, w, b, w};
//    orderedKeys = orderedKeys + orderedKeys + orderedKeys[0];
//    for (auto type: orderedKeys)
//        keys.push_back(SynthKey(type));
//
//    repositionKeys();
//}
//
//void SynthKeyboard::onMidiEvent(const MidiEvent & event)
//{
//	unsigned char eventType = event.getMessage()[0];
//	unsigned char keyCode = event.getMessage()[1];
//	unsigned char intensity = event.getMessage()[2];
//
//	static int msgId = 0;
//	std::cout << msgId++ << event.getMessage().size() << ": " <<
//		std::bitset<8>(eventType) << " " <<
//		std::bitset<8>(keyCode) << " " <<
//		std::bitset<8>(intensity) << "\n";
//
//	enum class MsgType : uint8_t {
//		KEYDOWN = 0b1001'0000,
//		KEYUP = 0b1000'0000,
//		KNOB = 0b1011'0000,
//		WHEEL = 0b1110'0000,
//	};
//
//	unsigned char value = keyCode - 48;
//	if (value >= keys.size())
//		return;
//
//	eventType &= 0b1111'0000;
//	switch (static_cast<MsgType>(eventType))
//	{
//	case MsgType::KEYDOWN:
//		onKey(value, SynthKey::State::Pressed);
//		break;
//	case MsgType::KEYUP:
//		onKey(value, SynthKey::State::Released);
//		break;
//	default:
//		break;
//	}
//}
//
//void SynthKeyboard::onSfmlEvent(const sf::Event & event)
//{
//	const auto& key = event.key.code;
//
//	int value = -1;
//	switch (key)
//	{
//	case sf::Keyboard::Z:
//		value = 0;
//		break;
//	case sf::Keyboard::S:
//		value = 1;
//		break;
//	case sf::Keyboard::X:
//		value = 2;
//		break;
//	case sf::Keyboard::D:
//		value = 3;
//		break;
//	case sf::Keyboard::C:
//		value = 4;
//		break;
//	case sf::Keyboard::V:
//		value = 5;
//		break;
//	case sf::Keyboard::G:
//		value = 6;
//		break;
//	case sf::Keyboard::B:
//		value = 7;
//		break;
//	case sf::Keyboard::H:
//		value = 8;
//		break;
//	case sf::Keyboard::N:
//		value = 9;
//		break;
//	case sf::Keyboard::J:
//		value = 10;
//		break;
//	case sf::Keyboard::M:
//		value = 11;
//		break;
//	default:
//		break;
//	}
//
//	if (value == -1)
//		return;
//
//	switch (event.type)
//	{
//	case sf::Event::KeyPressed: {
//		onKey(value, SynthKey::State::Pressed);
//		break;
//	}
//	case sf::Event::KeyReleased: {
//		onKey(value, SynthKey::State::Released);
//		break;
//	}
//	default:
//		break;
//	}
//}
//
//void SynthKeyboard::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
//{
//    for (const auto& key: keys)
//        if (key.type == SynthKey::White)
//            target.draw(key, states);
//
//    for (const auto& key: keys)
//        if (key.type == SynthKey::Black)
//            target.draw(key, states);
//}
//
//SynthRect SynthKeyboard::AABB() const
//{
//	SynthVec2 size;
//	size.x = keys.back().getPosition().x - keys.front().getPosition().x + keys.back().getSize().x;
//	size.y = std::max(whiteSize.y, blackSize.y);
//	return SynthRect{ SynthVec2(getPosition()), size };
//}
//
//bool SynthKeyboard::needsEvent(const SynthEvent & event) const
//{
//	if (std::holds_alternative<MidiEvent>(event)) return true;
//	const auto& sfEvent = std::get<sf::Event>(event);
//	if (sfEvent.type == sf::Event::KeyPressed ||
//		sfEvent.type == sf::Event::KeyReleased)
//		return true;
//	return false;
//}
//
//void SynthKeyboard::repositionKeys()
//{
//    unsigned whitesCount = 0;
//    SynthFloat dif = whiteSize.x - blackSize.x / 2;
//    for (unsigned i=0; i<keys.size(); ++i) {
//        auto& key = keys[i];
//        if (key.type == SynthKey::White) {
//            key.setPosition(whitesCount* key.getSize().x, 0);
//            ++whitesCount;
//        }
//        else {
//            key.setPosition(keys[i-1].getPosition().x + dif, 0);
//        }
//    }
//}
//
//void Slider::refreshText()
//{
//	std::ostringstream oss;
//	oss << std::setprecision(2) << std::fixed << value;
//	title.setText(name + "\n[" + oss.str() + "]");
//	const auto& s = mainRect.getSize();
//	const auto& p = SynthVec2(0, 0);
//	title.moveAroundPoint({ p.x + s.x / 2.f, p.y - 20 });
//}
//
//Slider::Slider(const std::string& str, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::function<void()> callback)
//	:title(str, 0, 0, 0, 0, titleSize), from(from), to(to), name(str), orientation(ori), value((from + to) / 2.), onMove(callback), size(sx, sy)
//{
//	const auto& minDim = std::min(sx, sy) / 2;
//	sliderRectSize = SynthVec2(minDim, minDim);
//
//	if (orientation == Vertical) {
//		mainRect.setSize(sf::Vector2f(size));
//	}
//	else {
//		mainRect.setSize(sf::Vector2f(size.y, size.x));
//	}
//
//	setPosition( px, py );
//
//	mainRect.setPosition({ 0,0 });
//    mainRect.setOutlineColor(sf::Color(0x757575FF));
//    mainRect.setFillColor(sf::Color(0x660000FF));
//    mainRect.setOutlineThickness(1.);
//
//	sliderRect.setSize(sf::Vector2f(sliderRectSize));
//    sliderRect.setPosition(mainRect.getSize().x/2 - sliderRectSize.x/2, mainRect.getSize().y/2 - sliderRectSize.y/2);
//    sliderRect.setOutlineColor(sf::Color(0x757575FF));
//    sliderRect.setFillColor(sf::Color(0x003366FF));
//    sliderRect.setOutlineThickness(2.);
//
//	refreshText();
//}
//
//Slider::Slider(const std::string& str, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::atomic<double>& val)
//    : Slider(str, from, to, px, py, sx, sy, titleSize, ori, [&](){val=getValue();})
//{
//}
//
//bool Slider::needsEvent(const SynthEvent & event) const
//{
//	if (std::holds_alternative<MidiEvent>(event)) return false;
//	const auto& sfEvent = std::get<sf::Event>(event);
//	if (sfEvent.type == sf::Event::MouseButtonPressed ||
//		sfEvent.type == sf::Event::MouseButtonReleased ||
//		sfEvent.type == sf::Event::MouseMoved)
//		return true;
//	return false;
//}
//
//void Slider::onSfmlEvent(const sf::Event& event)
//{
//	switch (event.type)
//	{
//		case sf::Event::MouseButtonPressed: {
//			const auto& mousePos = SynthVec2(event.mouseButton.x, event.mouseButton.y);
//			if (containsPoint(mousePos))
//				clicked = true;
//			break;
//		}
//		case sf::Event::MouseButtonReleased: {
//			clicked = false;
//			if (fixed) {
//				auto[px, py] = mainRect.getPosition();
//				sliderRect.setPosition(px + mainRect.getSize().x / 2 - sliderRectSize.x / 2, py + mainRect.getSize().y / 2 - sliderRectSize.y / 2);
//				moveSlider(SynthVec2(px + mainRect.getSize().x / 2, py + mainRect.getSize().y / 2));
//				refreshText();
//				if (onMove) onMove();
//			}
//			break;
//		}
//		case sf::Event::MouseMoved: {
//			if (clicked) {
//				const auto& mousePos = globalTransform.getInverse() * sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
//				moveSlider(SynthVec2(mousePos));
//				refreshText();
//				if (onMove) onMove();
//			}
//			break;
//		}
//		default: {
//			break;
//		}
//	}
//}
//
//SynthRect Slider::AABB() const
//{
//	SynthRect 
//		box1{ title.AABB() }, 
//		box2{ SynthVec2(getPosition()), SynthVec2(mainRect.getSize()) };
//	box1.left += getPosition().x;
//	box1.top += getPosition().y;
//	SynthFloat left = std::min(box1.left, box2.left);
//	SynthFloat top = std::min(box1.top, box2.top);
//	SynthFloat right = std::max(box1.left + box1.width, box2.left + box2.width);
//	SynthFloat bot = std::max(box1.top + box1.height, box2.top + box2.height);
//	return { left, top, right - left, bot - top };
//}
//
//void Slider::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
//{
//    target.draw(title, states);
//    target.draw(mainRect, states);
//    target.draw(sliderRect, states);
//}
//
//bool Slider::containsPoint(const SynthVec2& p) const
//{
//    const auto& rect = sliderRect;
//    const auto& u = globalTransform * rect.getPosition();
//    const auto& v = u + rect.getSize();
//
//    return u.x <= p.x && p.x <= v.x &&
//        u.y <= p.y && p.y <= v.y;
//}
//
//void Slider::moveSlider(const SynthVec2& p)
//{
//    const auto& minPos = mainRect.getPosition();
//    const auto& maxPos = minPos + mainRect.getSize() - sliderRect.getSize();
//    const auto& rectSize = maxPos - minPos;
//    const auto& currentPos = sliderRect.getPosition();
//    const auto& sliderSize = sliderRect.getSize();
//    const auto& midPos = minPos + rectSize / 2.f;
//
//    double newValueNormalized;
//
//    if (orientation == Vertical) {
//        sliderRect.setPosition(currentPos.x, std::clamp(p.y-sliderSize.y/2, SynthFloat(minPos.y), SynthFloat(maxPos.y)));
//        newValueNormalized = (midPos.y - sliderRect.getPosition().y) / rectSize.y*2;
//    }
//    else {
//        sliderRect.setPosition(std::clamp(p.x-sliderSize.x/2, SynthFloat(minPos.x), SynthFloat(maxPos.x)), currentPos.y);
//        newValueNormalized = (sliderRect.getPosition().x - midPos.x) / rectSize.x*2;
//    }
//
//    value = from + (newValueNormalized+1) / 2. * (to-from);
//}
//
//Oscilloscope::Oscilloscope(SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned res, double speed)
//    : resolution(res), speed(speed)
//{
//    setPosition(px, py);
//
//    window.setSize(sf::Vector2f(sx, sy));
//    window.setOutlineColor(sf::Color::White);
//    window.setFillColor(sf::Color::Black);
//    window.setOutlineThickness(2.);
//
//    vArray.reserve(resolution*2); // bigger buffer size for faster incoming samples
//    vArray.resize(resolution);
//    const auto& pos = window.getPosition();
//    const auto& size = window.getSize();
//    for (unsigned i=0; i<resolution; ++i) {
//        vArray[i].position.x = pos.x+i*size.x/resolution;
//        vArray[i].position.y = pos.y+size.y/2;
//        vArray[i].color = sf::Color::Green;
//    }
//}
//
//SynthRect Oscilloscope::AABB() const
//{
//	return { SynthVec2(getPosition()), SynthVec2(window.getSize()) };
//}
//
//void Oscilloscope::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
//{
//    target.draw(window, states);
//    target.draw(vArray.data(), resolution, sf::LineStrip, states);
//}
//
//void Oscilloscope::newSamples(const std::vector<double>& samples) const
//{
//	const double halfY = window.getSize().y / 2;
//    const unsigned dif = samples.size();
//    if (dif < vArray.size()) {
//        for (auto i=vArray.begin(); i<vArray.end()-dif; ++i)
//            i->position.y = (i+dif)->position.y;
//		const std::size_t shift = vArray.size() + dif;
//        for (auto i=vArray.end()-dif; i<vArray.end(); ++i)
//			i->position.y = window.getPosition().y + halfY + samples[i-vArray.begin()- shift] * halfY;
//    }
//    else {
//        for (unsigned i=0; i<vArray.size(); ++i)
//            vArray[i].position.y = window.getPosition().y + halfY + samples[i] * halfY;
//    }
//}
//
//const SynthVec2 TextDisplay::topLeftAlignment() const
//{
//	const auto& boundingBox = text.getGlobalBounds();
//	const auto& topleft = sf::Vector2f( boundingBox.left, boundingBox.top );
//	return SynthVec2(topleft - text.getPosition());
//}
//
//void TextDisplay::centralize()
//{
//	const auto& textBounds = text.getGlobalBounds();
//	const auto& textDim    = sf::Vector2f( textBounds.width, textBounds.height );
//	const auto& topLeftPos = sf::Vector2f( textBounds.left, textBounds.top );
//
//	auto dif = (frame.getSize() - textDim) / 2.f - (topLeftPos - text.getPosition());
//
//	dif.x = std::round(dif.x);
//	dif.y = std::round(dif.y);
//
//	text.setPosition(dif);
//}
//
//void TextDisplay::setFixedSize(bool fixed)
//{
//	fixedFrame = fixed;
//}
//
//void TextDisplay::setFrameSize(const SynthVec2 & size)
//{
//	if (fixedFrame) return;
//	frame.setSize(sf::Vector2f(size));
//}
//
//void TextDisplay::fitFrame(const SynthVec2& size)
//{
//	if (fixedFrame) return;
//	const auto& boundingBox = SynthRect(text.getGlobalBounds());
//	frame.setSize(sf::Vector2f(
//		std::max(size.x, boundingBox.width), 
//		std::max(size.y, boundingBox.height) 
//	));
//}
//
//TextDisplay::TextDisplay(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int size)
//    : text()
//{
//    text.setFont(font);
//    text.setCharacterSize(size);
//    text.setString(initialText);
//    text.setPosition(std::round(px), std::round(py));
//    text.setFillColor(sf::Color::Green);
//
//	frame.setOutlineThickness(0);
//	frame.setFillColor(sf::Color::Transparent);
//	frame.setPosition(0, 0);
//
//	fitFrame({ sx, sy });
//
//	const auto& boundingBox = text.getGlobalBounds();
//	setPosition(boundingBox.left, boundingBox.top);
//
//	text.setPosition(sf::Vector2f(-topLeftAlignment()));
//}
//
//std::unique_ptr<TextDisplay> TextDisplay::DefaultText(
//	const std::string & initialText, 
//	SynthFloat px, SynthFloat py, 
//	unsigned int charSize)
//{
//	return std::make_unique<TextDisplay>(initialText, px, py, 0, 0, charSize);
//}
//
//std::unique_ptr<TextDisplay> TextDisplay::Multiline(
//	const std::string initialText, 
//	SynthFloat px, 
//	SynthFloat py, 
//	SynthFloat width, 
//	unsigned int charSize
//)
//{
//	sf::Text dummyText;
//	dummyText.setFont(font);
//	dummyText.setCharacterSize(charSize);
//	dummyText.setPosition(px, py);
//
//	std::string row, result;
//	std::istringstream words(initialText);
//	auto wordIt = std::istream_iterator<std::string>(words);
//	result = *wordIt;
//	row = *wordIt;
//	unsigned wordCount = 1;
//	while(++wordCount, ++wordIt != std::istream_iterator<std::string>()) {
//		row.append(" "+*wordIt);
//		dummyText.setString(row);
//		const auto& bounds = dummyText.getGlobalBounds();
//		if (wordCount > 1 && bounds.width > width) {
//			result.append("\n");
//			row = *wordIt;
//			wordCount = 1;
//		}
//		else {
//			result.append(" ");
//		}
//		result.append(*wordIt);
//	}
//
//	return TextDisplay::DefaultText(result, px, py, charSize);
//}
//
//void TextDisplay::setBgColor(const sf::Color& color)
//{
//	frame.setFillColor(color);
//}
//
//const std::string & TextDisplay::getText() const
//{
//	return content;
//}
//
//const sf::Color & TextDisplay::getTextColor() const
//{
//	return text.getFillColor();
//}
//
//const sf::Color& TextDisplay::getBgColor() const
//{ 
//	return frame.getFillColor();
//}
//
//SynthRect TextDisplay::AABB() const
//{
//	return { SynthVec2(getPosition()), SynthVec2(frame.getSize()) };
//}
//
//void TextDisplay::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
//{
//	target.draw(frame, states);
//    target.draw(text, states);
//}
//
//void TextDisplay::setText(const std::string& content)
//{
//    text.setString(content);
//	fitFrame();
//}
//
//void TextDisplay::setTextColor(const sf::Color & color)
//{
//	text.setFillColor(color);
//}
//
//Button::Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick)
//    :TextDisplay(initialText, px, py, sx, sy, charSize), clickCallback(onClick)
//{
//	centralize();
//	frame.setOutlineColor(sf::Color::White);
//	frame.setOutlineThickness(1);
//}
//
//bool Button::needsEvent(const SynthEvent & event) const
//{
//	if (std::holds_alternative<MidiEvent>(event)) return false;
//	const auto& sfEvent = std::get<sf::Event>(event);
//	if (sfEvent.type == sf::Event::MouseButtonPressed || 
//		sfEvent.type == sf::Event::MouseButtonReleased)
//		return true;
//	return false;
//}
//
//
//void Button::onSfmlEvent(const sf::Event& event)
//{
//	static bool black = true;
//	if (event.type == sf::Event::MouseButtonPressed) {
//		SynthRect rect = { SynthVec2(globalTransform * frame.getPosition()), SynthVec2(frame.getSize()) };
//		if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
//			black = !black;
//			frame.setFillColor(black ? sf::Color::Black : sf::Color(0xaaaaaaaa));
//			clickCallback();
//		}
//	}
//}
//
//Window::Window(const std::string& title, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, const sf::Color & fillColor)
//	:mainRect( sf::Vector2f(sx, sy) )
//{
//	setDynamic(true);
//	setPosition(px, py);
//	mainRect.setPosition(0, headerSize);
//	mainRect.setFillColor(fillColor);
//	
//	mainRect.setOutlineThickness(0);
//	mainRect.setOutlineColor(sf::Color::Green);
//
//	header.setTextColor(sf::Color::Black);
//	header.setText(title);
//	header.setBgColor(sf::Color::White);
//	header.setPosition(0, 0);
//	header.setFrameSize(SynthVec2(sx, headerSize));
//	header.setFixedSize(true);
//	header.centralize();
//}
//
//const sf::FloatRect Window::globalMainRect() const
//{
//	return globalTransform.transformRect(sf::FloatRect(
//		mainRect.getPosition(), 
//		mainRect.getSize()
//	));
//}
//
//bool Window::needsEvent(const SynthEvent & event) const
//{
//	if (auto pEvent = std::get_if<sf::Event>(&event)) {
//		if (pEvent->type == sf::Event::MouseMoved) {
//			const auto& rect = globalMainRect();
//			if (!rect.contains(sf::Vector2f(pEvent->mouseMove.x, pEvent->mouseMove.y)) && 
//				rect.contains(sf::Vector2f(lastMousePos))) {
//				return false;
//			}
//		}
//	}
//	return true;
//}
//
//bool Window::forwardsEvent(const SynthEvent& event) const
//{
//	if (auto pEvent = std::get_if<sf::Event>(&event)) {
//		if (pEvent->type != sf::Event::MouseButtonPressed)
//			return true;
//
//		else if (globalMainRect().contains(sf::Vector2f(pEvent->mouseButton.x, pEvent->mouseButton.y))) {
//			return true;
//		}
//
//		else
//			return false;
//	}
//	else return true;
//}
//
//void Window::onSfmlEvent(const sf::Event & event)
//{
//	switch (event.type) {
//		case sf::Event::MouseButtonPressed: {
//			SynthRect rect = { SynthVec2(globalTransform * header.getPosition()), SynthVec2(mainRect.getSize().x, headerSize) };
//			lastMousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
//			if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
//				moving = true;
//			}
//			break;
//		}
//		case sf::Event::MouseButtonReleased: {
//			moving = false;
//			break;
//		}
//		case sf::Event::MouseMoved: {
//			if (moving) {
//				const auto& current = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
//				const auto& dif = sf::Vector2f(current - lastMousePos);
//				move(dif);
//				lastMousePos = current;
//			}
//			break;
//		}
//	}
//	if (event.type == sf::Event::MouseButtonPressed) {
//		SynthRect rect = { SynthVec2(globalTransform * header.getPosition()), SynthVec2(mainRect.getSize().x, headerSize) };
//
//		if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
//			moving = true;
//		}
//	}
//	else if (event.type == sf::Event::MouseButtonReleased) {
//		moving = false;
//	}
//}
//
//void Window::drawImpl(sf::RenderTarget & target, sf::RenderStates states) const
//{
//	target.draw(mainRect, states);
//	target.draw(header, states);
//}
//
//sf::View Window::childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const
//{
//	return getCroppedView(
//		target.getView(),
//		SynthVec2(states.transform.transformPoint(mainRect.getPosition())),
//		SynthVec2(mainRect.getSize())
//	);
//}
//
//void Window::setSize(const SynthVec2 & size)
//{
//	mainRect.setSize(sf::Vector2f(size));
//}
//
//const SynthVec2& Window::getSize()
//{
//	return std::move(SynthVec2(mainRect.getSize()));
//}
//
//void Window::addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children)
//{
//	addChildren(children);
//}
//
//SynthRect Window::AABB() const
//{
//	return { 
//		SynthVec2(getPosition()), 
//		SynthVec2(mainRect.getSize() + sf::Vector2f(0, header.AABB().height)) 
//	};
//}
