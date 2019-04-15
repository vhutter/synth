#include "Frame.h"

Frame::Frame(const SynthFloat sx, const SynthFloat sy)
	:frame(sf::Vector2f(sx, sy))
{
}

Frame::Frame(const SynthVec2 & size)
	:frame(sf::Vector2f(size))
{
}

void Frame::fitToChildren()
{
	float x1 = getPosition().x;
	float y1 = getPosition().y;
	float x2 = x1 + getSize().x;
	float y2 = x1 + getSize().y;
	for (auto child : children) {
		const auto& aabb = child->AABB();
		float cx1 = aabb.left;
		float cy1 = aabb.top;
		float cx2 = cx1 + aabb.width;
		float cy2 = cy1 + aabb.height;
		x1 = std::min(x1, cx1);
		y1 = std::min(y1, cy1);
		x2 = std::max(x2, cx2);
		y2 = std::max(y2, cy2);
	}
	setPosition(x1, y1);
	setSize({ x2 - x1, y2 - y1 });
}

void Frame::setChildAlignment(unsigned a)
{
	childAlignment = a;
}

void Frame::setCursor(unsigned x, unsigned y)
{
	cursorX = x;
	cursorY = y;
}

void Frame::addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children)
{
	addChildren(children);
	for (auto child : children) {
		child->setPosition(cursorX, cursorY);
		const auto& aabb = child->AABB();
		cursorX += aabb.width + childAlignment;
		if (cursorX > getSize().x) {
			cursorX = 0;
			cursorY += rowHeight + childAlignment;
			rowHeight = 0;
			child->setPosition(cursorX, cursorY);
		}
		rowHeight = std::max(rowHeight, unsigned(aabb.height));
	}
}

void Frame::setCropping(bool crop)
{
	cropping = crop;
}

const sf::Color & Frame::getBgColor() const
{
	return frame.getFillColor();
}

const sf::Color & Frame::getOutlineColor() const
{
	return frame.getOutlineColor();
}

const SynthFloat Frame::getOutlineThickness() const
{
	return SynthFloat(frame.getOutlineThickness());
}

const SynthVec2 Frame::getSize() const
{
	return SynthVec2(frame.getSize());
}

void Frame::setBgColor(const sf::Color & color)
{
	frame.setFillColor(color);
}

void Frame::setOutlineColor(const sf::Color & color)
{
	frame.setOutlineColor(color);
}

void Frame::setOutlineThickness(SynthFloat thickness)
{
	frame.setOutlineThickness(thickness);
}

void Frame::setSize(const SynthVec2 & size)
{
	frame.setSize(sf::Vector2f(size));
}

SynthRect Frame::AABB() const
{
	return SynthRect(
		SynthVec2(getPosition()),
		getSize()
	);
}

void Frame::drawImpl(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(frame, states);
}

sf::View Frame::childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const
{
	if (cropping)
		return getCroppedView(target.getView(), SynthRect(SynthVec2(states.transform.transformPoint(0,0)), getSize()));
	else
		return GuiElement::childrenView(target, states);
}

const sf::FloatRect Frame::globalFrame() const
{
	return globalTransform.transformRect(sf::FloatRect(
		frame.getPosition(),
		sf::Vector2f(frame.getSize())
	));
}

bool Frame::needsEvent(const SynthEvent & event) const
{
	if (!cropping)
		return true;

	if (auto pEvent = std::get_if<sf::Event>(&event)) {
		if (pEvent->type == sf::Event::MouseMoved) {
			const auto& rect = globalFrame();
			if (!rect.contains(sf::Vector2f(pEvent->mouseMove.x, pEvent->mouseMove.y))) {
				return false;
			}
		}
	}
	return true;
}

bool Frame::forwardsEvent(const SynthEvent& event) const
{
	if (!cropping)
		return true;

	if (auto pEvent = std::get_if<sf::Event>(&event)) {
		if (pEvent->type != sf::Event::MouseButtonPressed)
			return true;
		else if (globalFrame().contains(sf::Vector2f(pEvent->mouseButton.x, pEvent->mouseButton.y)))
			return true;
		else
			return false;
	}
	else
		return true;
}
