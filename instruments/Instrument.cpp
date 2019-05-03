#include "Instrument.h"

void Instrument1::init()
{
	using pos_t = MenuOption::OptionList::ChildPos_t;
	auto gui = window->getContentFrame();
	gui->setChildAlignment(10);
	gui->setCursor(10, 10);

	keyboard.outputTo(
		generator,
		glider
	);

	gui->addChildAutoPos(pitchBender.getFrame());
	gui->addChildAutoPos(glider.getFrame());

	generator.addAfterCallback(glider);

	auto kbAABB = keyboard.getSynthKeyboard()->AABB();
	gui->addChild(keyboard.getSynthKeyboard(), 0, wHeight - kbAABB.height);

	window->addEmptyListener(std::make_unique<EmptyGuiElement>([this](const sf::Event & event) {
		if (event.type == sf::Event::KeyPressed) {

			auto synthKeyboard = keyboard.getSynthKeyboard();
			unsigned shift = synthKeyboard->getOctaveShift();
			switch(event.key.code) {
				case sf::Keyboard::Up:
					keyboard.stopAll();
					++shift;
					break;
				case sf::Keyboard::Down:
					keyboard.stopAll();
					--shift;
					break;
				default:
					break;
			};
			synthKeyboard->setOctaveShift(shift);
		}
	}));

	gui->fitToChildren();
	window->setSize(gui->getSize());
	window->setMenuBar(menuHeight);
	window->setOnClose([this]() {keyboard.stopAll(); });
	window->getMenuFrame()->addChildAutoPos(MenuOption::createMenu(
		30, 15, {
			"Settings", pos_t::Down, {
			}
		}
	));
}
