#include "Slider.h"
#include "TextDisplay.h"

#include <string>
#include <sstream>
#include <iomanip>

void Slider::refreshText()
{
	using namespace std::string_literals;
	std::ostringstream oss;
	oss << std::setprecision(2) << std::fixed << value;
	titleText->setText(name);
	valueText->setText(oss.str());
	unsigned textSize = titleText->getTextSize();
	const auto& s = mainRect.getSize();
	const auto& p = mainRect.getPosition();
	titleText->moveAroundPoint({ std::round(p.x + s.x / 2.f), std::round(p.y - textSize * 2) });
	valueText->moveAroundPoint({ std::round(p.x + s.x / 2.f), std::round(p.y - textSize * 1) });
}

Slider::Slider(const std::string& str, double from, double to, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::function<void()> callback)
	:titleText(std::make_unique<TextDisplay>(str, 0, 0, titleSize)), 
	valueText(std::make_unique<TextDisplay>(str, 0, 0, titleSize)), 
	from(from), 
	to(to), 
	name(str), 
	orientation(ori), 
	value((from + to) / 2.), 
	onMove(callback), 
	size(sx, sy)
{
	const auto& minDim = std::min(sx, sy) / 2;
	sliderRectSize = SynthVec2(minDim, minDim);

	if (orientation == Vertical) {
		mainRect.setSize(sf::Vector2f(size));
	}
	else {
		mainRect.setSize(sf::Vector2f(size.y, size.x));
	}

	mainRect.setPosition({ 0,0 });
	mainRect.setOutlineColor(sf::Color(0x757575FF));
	mainRect.setFillColor(sf::Color(0x660000FF));
	mainRect.setOutlineThickness(-1.);

	sliderRect.setSize(sf::Vector2f(sliderRectSize));
	sliderRect.setPosition(mainRect.getSize().x / 2 - sliderRectSize.x / 2, mainRect.getSize().y / 2 - sliderRectSize.y / 2);
	sliderRect.setOutlineColor(sf::Color(0x757575FF));
	sliderRect.setFillColor(sf::Color(0x003366FF));
	sliderRect.setOutlineThickness(-2.);

	refreshText();
	const auto& aabb = AABB();
	const auto& currentPos = sf::Vector2f(aabb.left, aabb.top);

	titleText->move(-currentPos);
	valueText->move(-currentPos);
	mainRect.move(-currentPos);
	sliderRect.move(-currentPos);
	//refreshText();
}

Slider::Slider(const std::string& str, double from, double to, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::atomic<double>& val)
	: Slider(str, from, to, sx, sy, titleSize, ori, [&]() {val = getValue(); })
{
}

bool Slider::needsEvent(const SynthEvent & event) const
{
	if (std::holds_alternative<MidiEvent>(event)) return false;
	const auto& sfEvent = std::get<sf::Event>(event);
	if (sfEvent.type == sf::Event::MouseButtonPressed ||
		sfEvent.type == sf::Event::MouseButtonReleased ||
		sfEvent.type == sf::Event::MouseMoved)
		return true;
	return false;
}

void Slider::onSfmlEvent(const sf::Event& event)
{
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

SynthRect Slider::AABB() const
{
	SynthRect
		box1{ titleText->AABB() },
		box2{ valueText->AABB() },
		box3{ SynthVec2(mainRect.getPosition()), SynthVec2(mainRect.getSize()) };
	SynthFloat left = std::min({ box1.left, box2.left, box3.left });
	SynthFloat top = std::min({ box1.top, box2.top, box3.top });
	SynthFloat right = std::max({ box1.left + box1.width, box2.left + box2.width, box3.left + box3.width });
	SynthFloat bot = std::max({ box1.top + box1.height, box2.top + box2.height, box3.top + box3.height });
	return { left, top, right - left, bot - top };
}

void Slider::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(*titleText, states);
	target.draw(*valueText, states);
	target.draw(mainRect, states);
	target.draw(sliderRect, states);
}

bool Slider::containsPoint(const SynthVec2& p) const
{
	const auto& u = globalTransform * sliderRect.getPosition();
	const auto& v = u + sliderRect.getSize();

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
		sliderRect.setPosition(currentPos.x, std::clamp(p.y - sliderSize.y / 2, SynthFloat(minPos.y), SynthFloat(maxPos.y)));
		newValueNormalized = (midPos.y - sliderRect.getPosition().y) / double(rectSize.y) * 2;
	}
	else {
		sliderRect.setPosition(std::clamp(p.x - sliderSize.x / 2, SynthFloat(minPos.x), SynthFloat(maxPos.x)), currentPos.y);
		newValueNormalized = (sliderRect.getPosition().x - midPos.x) / double(rectSize.x) * 2;
	}

	value = from + (newValueNormalized + 1) / 2. * (to - from);
}