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

	struct OptionList {
		enum class ChildPos_t{ Right, Down, /*Left, Up*/};
		
		const std::string title;
		const ChildPos_t childPos;
		const std::vector<OptionList> children;
	};
	static std::unordered_map<std::string, std::shared_ptr<GuiElement>> createMenu(
		std::shared_ptr<GuiElement> parent,
		const SynthVec2& pos,
		const SynthVec2& size,
		const OptionList option
	);

private:
	bool active{ false };
};
 
#endif // MENUOPTION_H