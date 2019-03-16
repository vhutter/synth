#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <memory>
#include <functional>
#include "guiElements.h"

void setupGui(GuiElement& gui, sf::RenderWindow& window);

class KeyboardOutput
{
public:
	KeyboardOutput();
	std::shared_ptr<SynthKeyboard> getGuiElement() const;

	template<typename ...T>
	void outputTo(T&... other)
	{
		using namespace std::placeholders;
		(callbacks.push_back(std::bind(&T::onKeyEvent, &other, _1, _2)), ...);
	}

private:
	std::vector<SynthKeyboard::callback_t> callbacks;
	std::shared_ptr<SynthKeyboard> kb;
};

#endif // GUI_H_INCLUDED