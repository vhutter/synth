#ifndef MENUOPTION_H
#define MENUOPTION_H

#include "Button.h"

class MenuOption :public Button
{
public:
	MenuOption(const std::string& text, unsigned int charSize = 0);
	void addChildren(const std::vector<std::shared_ptr<GuiElement>>& children);
	void toggle();
	bool isActive() const;

private:
	bool active{ false };
};
 
#endif // MENUOPTION_H