#ifndef TEXTDISPLAY_H_INCLUDED
#define TEXTDISPLAY_H_INCLUDED

#include "GuiElement.h"

class TextDisplay : public GuiElement
{
public:
	TextDisplay(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx = 0, SynthFloat sy = 0, unsigned int charSize = 30);
	TextDisplay() : TextDisplay("", 0, 0, 0, 0) {}
	static std::unique_ptr<TextDisplay> DefaultText(
		const std::string& initialText,
		SynthFloat px, SynthFloat py,
		unsigned int charSize = 30
	);
	static std::unique_ptr<TextDisplay> Multiline(
		const std::string text,
		SynthFloat px, SynthFloat py,
		SynthFloat width,
		unsigned int charSize = 24
	);

	const std::string& getText() const;
	const sf::Color& getTextColor() const;
	const sf::Color& getBgColor() const;
	const sf::Color& getOutlineColor() const;
	const SynthVec2 getFrameSize() const;
	const unsigned getTextSize() const;

	void setText(const std::string& text);
	void setTextColor(const sf::Color& color);
	void setBgColor(const sf::Color& color);
	void setOutlineColor(const sf::Color& color);
	void setFrameSize(const SynthVec2& size);
	void setTextSize(unsigned newSize);

	virtual SynthRect AABB() const override;
	void centralize();
	void setFixedSize(bool fixed);
	void fitFrame(const SynthVec2& size = { 0,0 });

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	const SynthVec2 topLeftAlignment() const;

	sf::RectangleShape frame;
	sf::Text text;
	std::string content;
	bool fixedFrame{ false };

	static sf::Font font;
};

#endif //TEXTDISPLAY_H_INCLUDED