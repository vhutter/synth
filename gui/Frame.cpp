#include "Frame.h"

Frame::Frame(const SynthFloat sx, const SynthFloat sy)
	:frame(sf::Vector2f(sx, sy))
{
}

Frame::Frame(const SynthVec2 & size)
	:frame(sf::Vector2f(size))
{
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
