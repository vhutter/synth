#include "MenuOption.h"

MenuOption::MenuOption(const std::string & text, unsigned int charSize)
	:Button(text, 0,0,0,0, charSize)
{
	//setFocusable(false);
	setEventCallback([this](const sf::Event& event) {
		if (event.type == sf::Event::MouseButtonPressed) {
			sf::Vector2f mPos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
			const auto& t = globalTransform;
			if (!globalTransform.transformRect({frame.getPosition(), sf::Vector2f(getSize())}).contains(mPos)) {
				if (active)
					for (auto child : children) {
						child->forwardEvent(event, globalTransform);
					}
				else 
					active = true; // because we want to deactivate with toggle()
			}
			toggle();
		}
	});
}

void MenuOption::addChildren(const std::vector<std::shared_ptr<GuiElement>>& children)
{
	GuiElement::addChildren(children);
	for (auto child : children) {
		child->setVisibility(active);
	}
}

void MenuOption::toggle()
{
	active = !active;
	for (auto child : children) {
		child->setVisibility(active);
	}
}

bool MenuOption::isActive() const
{
	return active;
}
