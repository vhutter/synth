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
	const auto& textBounds = SynthRect(text.getGlobalBounds());
	const auto& textDim = SynthVec2(textBounds.width, textBounds.height);
	const auto& topLeftPos = SynthVec2(textBounds.left, textBounds.top);

	auto dif = (getSize() - textDim) / SynthFloat(2) - (topLeftPos - SynthVec2(text.getPosition()));

	dif.x = std::round(dif.x);
	dif.y = std::round(dif.y);

	text.setPosition(sf::Vector2f(dif));
}

void TextDisplay::setFixedSize(bool fixed)
{
	fixedFrame = fixed;
}

void TextDisplay::fitFrame(const SynthVec2& size)
{
	if (fixedFrame) return;
	const auto& boundingBox = SynthRect(text.getGlobalBounds());
	setSize({
		std::max(size.x, boundingBox.width),
		std::max(size.y, boundingBox.height)
	});
}

TextDisplay::TextDisplay(const std::string& initialText, SynthFloat sx, SynthFloat sy, unsigned int size)
	: text()
{
	setFocusable(false);

	text.setFont(font);
	text.setCharacterSize(size);
	text.setString(initialText);
	text.setFillColor(sf::Color::Green);

	setOutlineThickness(0);
	setBgColor(sf::Color::Transparent);
	setPosition(0, 0);

	fitFrame({ sx, sy });

	const auto& boundingBox = text.getGlobalBounds();
	//setPosition(px, py);

	text.setPosition(sf::Vector2f(-topLeftAlignment()));
}

std::unique_ptr<TextDisplay> TextDisplay::DefaultText(
	const std::string & initialText,
	unsigned int charSize)
{
	return std::make_unique<TextDisplay>(initialText, 0, 0, charSize);
}

std::unique_ptr<TextDisplay> TextDisplay::Multiline(
	const std::string initialText,
	SynthFloat width,
	unsigned int charSize
)
{
	sf::Text dummyText;
	dummyText.setFont(font);
	dummyText.setCharacterSize(charSize);

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

	return TextDisplay::DefaultText(result, charSize);
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

const unsigned TextDisplay::getTextSize() const
{
	return text.getCharacterSize();
}

void TextDisplay::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
	Frame::drawImpl(target, states);
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