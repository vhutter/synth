#ifndef TEXTDISPLAY_H_INCLUDED
#define TEXTDISPLAY_H_INCLUDED

#include "Frame.h"

class TextDisplay : public Frame
{
public:
	TextDisplay(
		const std::string& initialText, 
		SynthFloat sx = 0, 
		SynthFloat sy = 0, 
		unsigned int charSize = 30, 
		const sf::Font& font = loadCourierNew()
	);
	TextDisplay() : TextDisplay("") {}
	static std::unique_ptr<TextDisplay> DefaultText(
		const std::string& initialText,
		unsigned int charSize = 30
	);
	static std::unique_ptr<TextDisplay> Multiline(
		const std::string text,
		SynthFloat width,
		unsigned int charSize = 24,
		const sf::Font& font = loadCourierNew()
	);

	template<class string_t=sf::String>
	const string_t getText() const
	{
		std::lock_guard lock(mtx);
		return text.getString();
	}
	const sf::Color& getTextColor() const;
	const unsigned getTextSize() const;

	void setText(const std::string& text);
	void setText(const sf::String& text);
	void setTextColor(const sf::Color& color);
	void setTextSize(unsigned newSize);

	void centralize();
	void setFixedSize(bool fixed);
	void fitFrame(const SynthVec2& size = { 0,0 });

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	const SynthVec2 topLeftAlignment() const;

	sf::Text text;
	bool fixedFrame{ false };

	const sf::Font& font;
	mutable std::mutex mtx;
};

#endif //TEXTDISPLAY_H_INCLUDED