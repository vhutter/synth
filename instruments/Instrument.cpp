#include "Instrument.h"

void Instrument1::init()
{
	const unsigned wWidth{ 800 }, wHeight{ 400 }, menuHeight{ 30 };
	using pos_t = MenuOption::OptionList::ChildPos_t;
	window->setSize(SynthVec2(wWidth, wHeight));
	window->setHeader(menuHeight, "Instrument1");
	window->setMenuBar(menuHeight);
	auto gui = window->getContentFrame();
	auto menu = window->getMenuFrame();
	gui->setChildAlignment(10);
	gui->setCursor(10, 10);

	keyboard.outputTo(
		generator,
		glider
	);

	gui->addChildAutoPos(pitchBender.getFrame());
	gui->addChildAutoPos(glider.getFrame());

	auto kbAABB = keyboard.getGuiElement()->AABB();
	gui->addChild(keyboard.getGuiElement(), 0, wHeight - kbAABB.height);

	menu->addChildAutoPos(MenuOption::createMenu(
		100, 30, 15, {
			"Settings", pos_t::Down, {
			}
		}
	));


	generator.addBeforeCallback(pitchBender);
	generator.addAfterCallback(glider);
}
