#include <sstream>
#include <iterator>

#include "TextDisplay.h"

sf::Font TextDisplay::font = loadCourierNew();

const SynthVec2 TextDisplay::topLeftAlignment() const
{
	const auto& boundingBox = text.getGlobalBounds();
	const auto& topleft = sf::Vector2f(boundingBox.left, boundingBox.top);
	return SynthVec2(topleft - text.getPosition());
}

void TextDisplay::centralize()
{
	const auto& textBounds = text.getGlobalBounds();
	const auto& textDim = sf::Vector2f(textBounds.width, textBounds.height);
	const auto& topLeftPos = sf::Vector2f(textBounds.left, textBounds.top);

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
	while (++wordCount, ++wordIt != std::istream_iterator<std::string>()) {
		row.append(" " + *wordIt);
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

void TextDisplay::setOutlineColor(const sf::Color & color)
{
	text.setOutlineColor(color);
}

void TextDisplay::setTextSize(unsigned newSize)
{
	text.setCharacterSize(newSize);
}

const std::string & TextDisplay::getText() const
{
	return content;
}

const sf::Color & TextDisplay::getTextColor() const
{
	return text.getFillColor();
}

const sf::Color& TextDisplay::getBgColor() const
{
	return frame.getFillColor();
}

const sf::Color & TextDisplay::getOutlineColor() const
{
	return text.getOutlineColor();
}

const SynthVec2 TextDisplay::getFrameSize() const
{
	return SynthVec2(frame.getSize());
}

const unsigned TextDisplay::getTextSize() const
{
	return text.getCharacterSize();
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

void TextDisplay::setTextColor(const sf::Color & color)
{
	text.setFillColor(color);
}