#include "MenuOption.h"
#include "Window.h"

void MenuOption::init()
{
	passesAllClicks = true;
}

MenuOption* MenuOption::pressedChild()
{
	for (auto child : children) {
		auto* c = dynamic_cast<MenuOption*>(child.get());
		if (c->isPressed()) {
			return c;
		}
		else {
			auto* cPressed = c->pressedChild();
			if (cPressed) return cPressed;
		}
	}
	return nullptr;
}

MenuOption::MenuOption(const std::string & text, unsigned int charSize)
	:Button(text, 0, 0, 0, 0, charSize, [this]() {
		// the button is getting released after a click
		if (isPressed()) {
			active = !active;
		}
		else {
			auto* pChild = pressedChild();
			if (!pChild) active = false;
			else if (pChild->children.size() == 0) {
				pChild->forwardEvent(lastEvent, globalTransform);
				active = false;
			}
		}

		toggle(active);
	})
{
	init();
}

MenuOption::MenuOption(const std::string& text, unsigned int charSize, std::shared_ptr<Window> popup)
	:Button(text, 0, 0, 0, 0, charSize, [this, popup]() {
		if (isPressed()) {
			popup->setVisibility(true);
		}
	})
{
	init();
}

void MenuOption::addChildren(const std::vector<std::shared_ptr<MenuOption>>& children)
{
	for (auto child : children) {
		GuiElement::addChildren({ child });
		child->setVisibility(active);
	}
}

void MenuOption::toggle(bool state)
{
	for (auto child : children) {
		if (auto q = dynamic_cast<MenuOption*>(child.get())) {
			q->setVisibility(state);
			active = state;
			if (state == false) {
				q->toggle(state);
			}
		}
	}
}

bool MenuOption::isActive() const
{
	return active;
}

std::shared_ptr<MenuOption> MenuOption::createMenu(
	unsigned w, unsigned h,
	unsigned fontSize,
	const OptionList& option
)
{
	std::shared_ptr<MenuOption> ret;
	if (std::holds_alternative< std::shared_ptr<Window>>(option.children)) {
		ret = std::make_shared<MenuOption>(option.title, fontSize, std::get<std::shared_ptr<Window>>(option.children));
	}
	else {
		ret = std::make_shared<MenuOption>(option.title, fontSize);
	}
	ret->setSize(SynthVec2(w,h));
	ret->centralize();
	using pos_t = OptionList::ChildPos_t;
	SynthVec2 startPos{ 0,0 }, difPos{ 0, SynthFloat(h) };
	if (option.childPos == pos_t::Right) {
		startPos.x += w;
	}
	else if (option.childPos == pos_t::Down) {
		startPos.y += h;
	}

	if (std::holds_alternative< std::vector<OptionList>>(option.children)) {
		auto children = std::get<std::vector<OptionList>>(option.children);
		for (auto child : children) {
			auto newItem = createMenu(w, h, fontSize, child);
			newItem->setPosition(startPos.x, startPos.y);
			ret->addChildren({ newItem });
			startPos += difPos;
		}
	}

	return ret;
}
