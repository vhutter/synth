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
	void addChildren(const std::vector<std::shared_ptr<MenuOption>>& children);
	void toggle(bool state);
	bool isActive() const;

	struct OptionList {
		enum class ChildPos_t{ Right, Down };
		
		const std::string title;
		const ChildPos_t childPos;
		const std::variant< std::vector<OptionList>, std::shared_ptr<Window> > children;

		OptionList(const std::string& title, ChildPos_t childPos, const std::vector<OptionList>& children)
			:title(title), childPos(childPos), children(children)
		{}
		OptionList(const std::string& title, const std::vector<OptionList>& children = {})
			:title(title), childPos(ChildPos_t::Right), children(children)
		{}
		OptionList(const std::string& title, std::shared_ptr<Window> window)
			:title(title), childPos{}, children(window)
		{}
	};

	static std::shared_ptr<MenuOption> createMenu(
		unsigned w, unsigned h,
		unsigned fontSize,
		const OptionList& option
	);

private:
	void init();
	MenuOption* pressedChild();

	bool active{ false };
};
 
#endif // MENUOPTION_H