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

std::unordered_map<std::string, std::shared_ptr<GuiElement>> MenuOption::createMenu(
	std::shared_ptr<GuiElement> parent,
	const SynthVec2& pos,
	const SynthVec2& size,
	const OptionList option
)
{
	auto ret = std::unordered_map<std::string, std::shared_ptr<GuiElement>>();

	auto button = std::make_shared<MenuOption>(option.title, size.y);
	button->setPosition(sf::Vector2f(pos));
	button->setSize(size);
	using pos_t = OptionList::ChildPos_t;
	SynthVec2 startPos{ 0,0 }, difPos{ 0, size.y };
	if (option.childPos == pos_t::Right) {
		startPos.x += size.x;
	}
	else if (option.childPos == pos_t::Down) {
		startPos.y += size.y;
	}
	for (auto child : option.children) {
		auto childMap = createMenu(button, startPos, size, child);
		ret.insert(childMap.begin(), childMap.end());
		startPos += difPos;
	}

	return ret;
}
