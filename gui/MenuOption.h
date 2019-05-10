#ifndef MENUOPTION_H
#define MENUOPTION_H

#include <variant>
#include "Button.h"

class Window;

class MenuOption :public Button
{
public:
	MenuOption(const std::string& text, unsigned int charSize);
	MenuOption(const std::string& text, unsigned int charSize, std::shared_ptr<Window> popup);
	void addChild(std::shared_ptr<MenuOption> child, unsigned px=0, unsigned py=0);
	void toggle(bool state);
	bool isActive() const;

	struct OptionList {
		enum class ChildPos_t{ Right, Down };
		
		const std::string title;
		const ChildPos_t childPos;
		const std::variant< std::vector<OptionList>, std::shared_ptr<Window> > children;

		OptionList(std::string title, ChildPos_t childPos, std::vector<OptionList> children)
			:title(std::move(title)), childPos(childPos), children(std::move(children))
		{}
		OptionList(std::string title, std::vector<OptionList> children = {})
			:title(std::move(title)), childPos(ChildPos_t::Right), children(std::move(children))
		{}
		OptionList(std::string title, std::shared_ptr<Window> window)
			:title(std::move(title)), childPos{}, children(window)
		{}
	};

	static std::shared_ptr<MenuOption> createMenu(
		unsigned height,
		unsigned fontSize,
		const OptionList& option,
		unsigned alignment = 10u,
		unsigned width = 0u
	);

private:
	static void correctWidth(TextDisplay& tile, unsigned h, unsigned alignment);

	void init();
	MenuOption* pressedChild();

	bool active{ false };
};
 
#endif // MENUOPTION_H