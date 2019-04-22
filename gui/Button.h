#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <atomic>

#include "TextDisplay.h"

class Button : public TextDisplay
{
public:
	Button(const std::string& initialText, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick);

	static std::unique_ptr<Button> DefaultButton(const std::string& s, std::function<void()> onClick) {
		return std::make_unique<Button>(s, 100, 30, 16, onClick);
	}
	static std::unique_ptr<Button> DefaultButton(const std::string& s, std::atomic<bool>& val) {
		return std::make_unique<Button>(s, 100, 30, 16, [&]() {val = !val; });
	}

	void setNormalColor(const sf::Color& col);
	void setPressedColor(const sf::Color& col);
	bool isPressed() const;

	virtual bool needsEvent(const SynthEvent& event) const override;

protected:
	bool isPressedOrReleased(const sf::Event& event) const;
	sf::Event lastEvent;
	bool passesAllClicks{ false };

private:
	using TextDisplay::setBgColor;
	void refreshCol();

	std::function<void()> clickCallback;
	bool pressed{ false };
	sf::Color normalCol{ sf::Color::Black }, pressedCol{ 0x555555ff };
};

#endif //BUTTON_H_INCLUDED