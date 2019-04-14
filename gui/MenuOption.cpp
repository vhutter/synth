#include "MenuOption.h"

MenuOption::MenuOption(const std::string & text, unsigned int charSize)
	:Button(text, 0,0,0,0, charSize, [this]() { dropdown(); })
{
}

void MenuOption::addChildren(const std::vector<std::shared_ptr<GuiElement>>& children)
{
	GuiElement::addChildren(children);
	for (auto child : children) {
		child->setVisibility(active);
	}
}

void MenuOption::dropdown()
{
	active = !active;
	for (auto child : children) {
		child->setVisibility(active);
	}
}