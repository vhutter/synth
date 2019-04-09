#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <atomic>

#include "GuiElement.h"
#include "TextDisplay.h"

class Button : public TextDisplay
{
public:
	Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick);
	virtual bool needsEvent(const SynthEvent& event) const override;

	static std::unique_ptr<Button> DefaultButton(const std::string& s, SynthFloat px, SynthFloat py, std::function<void()> onClick) {
		return std::make_unique<Button>(s, px, py, 100, 30, 16, onClick);
	}
	static std::unique_ptr<Button> DefaultButton(const std::string& s, SynthFloat px, SynthFloat py, std::atomic<bool>& val) {
		return std::make_unique<Button>(s, px, py, 100, 30, 16, [&]() {val = !val; });
	}

private:
	virtual void onSfmlEvent(const sf::Event& event) override;

	std::function<void()> clickCallback;
};

#endif //BUTTON_H_INCLUDED