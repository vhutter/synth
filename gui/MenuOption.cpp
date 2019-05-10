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
	:Button(text, 0, 0, charSize, [this]() {
		// the button is getting released after a click
		if (isPressed()) {
			active = !active;
		}
		else {
			auto* pChild = pressedChild();
			if (!pChild) active = false;
			else if (pChild->children.empty()) {
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
	:Button(text, 0, 0, charSize, [this, popup]() {
		if (isPressed()) {
			popup->setVisibility(true);
			popup->focus();
		}
	})
{
	init();
}

void MenuOption::addChild(std::shared_ptr<MenuOption> child, unsigned px, unsigned py)
{
	child->setVisibility(active);
	GuiElement::addChild( child, px, py );
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



void MenuOption::correctWidth(TextDisplay& tile, unsigned h, unsigned alignment)
{
	tile.setFixedSize(false);
	tile.fitFrame(tile.getSize());
	tile.setSize(SynthVec2(tile.getSize().x + SynthFloat(alignment) * 2, h));
	tile.centralize();
	tile.setFixedSize(true);
}

std::shared_ptr<MenuOption> MenuOption::createMenu(
	unsigned h,
	unsigned fontSize,
	const OptionList& option,
	unsigned alignment,
	unsigned width
)
{
	std::shared_ptr<MenuOption> ret;

	if (std::holds_alternative< std::shared_ptr<Window>>(option.children))
		ret = std::make_shared<MenuOption>(option.title, fontSize, std::get<std::shared_ptr<Window>>(option.children));
	else
		ret = std::make_shared<MenuOption>(option.title, fontSize);

	if (width)
		ret->setSize(SynthVec2(width, h));
	else
		correctWidth(*ret, h, alignment);

	using pos_t = OptionList::ChildPos_t;
	SynthVec2 startPos{ 0,0 }, difPos{ 0, SynthFloat(h) };
	if (option.childPos == pos_t::Right) {
		startPos.x += ret->getSize().x;
	}
	else if (option.childPos == pos_t::Down) {
		startPos.y += h;
	}

	if (std::holds_alternative< std::vector<OptionList>>(option.children)) {
		auto children = std::get<std::vector<OptionList>>(option.children);

		// Precalculate max width of the direct descendants with plain TextDisplay
		unsigned maxWidth = 0;
		for (auto child : children) {
			auto newItem = TextDisplay(child.title, 0, h, fontSize);
			correctWidth(newItem, h, alignment);
			unsigned newWidth = unsigned(newItem.getSize().x);
			maxWidth = newWidth > maxWidth ? newWidth : maxWidth;
		}

		// Recursively create the submenu (with the previously calculated width for the direct descendants)
		auto newSize = SynthVec2(maxWidth, h);
		for (auto child : children) {
			auto newItem = createMenu(h, fontSize, child, alignment, maxWidth);
			newItem->centralize();
			ret->addChild(newItem, startPos.x, startPos.y);
			startPos += difPos;
		}
	}

	return ret;
}
