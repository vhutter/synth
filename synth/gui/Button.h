#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <atomic>
#include <string>

#include "TextDisplay.h"

class Button : public TextDisplay
{
public:
	Button(std::string initialText, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick);

	static std::unique_ptr<Button> DefaultButton(const std::string& s, std::function<void()> onClick) {
		return std::make_unique<Button>(
			s, 
			getConfig("defaultButtonWidth"), 
			getConfig("defaultButtonHeight"),
			getConfig("defaultCharSize"), 
			onClick
		);
	}
	static std::unique_ptr<Button> DefaultButton(const std::string& s, std::atomic<bool>& val) {
		return DefaultButton(s, [&]() {val = !val; });
	}
	static std::unique_ptr<Button> OnOffButton(std::atomic<bool>& val, std::function<void(bool)> cb = {}) {
		auto ret = std::make_unique<Button>("",
			getConfig("defaultOnOffButtonSize"),
			getConfig("defaultOnOffButtonSize"),
			getConfig("defaultCharSize"), [&]() {}
		);

		using namespace std::string_literals;
		auto setState = [button = ret.get()](bool val) {
			if (val) {
				button->setText("On"s);
				button->setNormalColor(sf::Color(0xA52A2AFF));
			}
			else {
				button->setText("Off"s);
				button->setNormalColor(sf::Color::Black);
			}
			button->centralize();
		};

		setState(val);
		ret->clickCallback = [button = ret.get(), &val, setState, cb]() {
			setState(!val);
			if (cb) cb(!val);
			val = !val;
		};

		return ret;
	}

	void setNormalColor(const sf::Color& col);
	void setPressedColor(const sf::Color& col);
	bool isPressed() const;

	virtual bool needsEvent(const SynthEvent& event) const override;

protected:
	sf::Event lastEvent;
	bool passesAllClicks{ false };

private:
	bool isPressedOrReleased(const sf::Event& event) const;
	using TextDisplay::setBgColor;
	void refreshCol();

	std::function<void()> clickCallback;
	bool pressed{ false };
	sf::Color normalCol{ getConfig("defaultButtonNormalColor") }, 
		pressedCol{ getConfig("defaultButtonPressedColor") };
};

#endif //BUTTON_H_INCLUDED