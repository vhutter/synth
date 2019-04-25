#include "testMain.h"
#include "../gui/GuiElements.h"
#include "../gui.h"
#include "../instruments/effects.h"
#include "../instruments/generators.h"
#include "../instruments/tones.h"

void testGui(std::shared_ptr<GuiElement> gui)
{
	///// Window
	sf::Color bgcolor = sf::Color::Magenta;
	bgcolor.a = 100;
	std::shared_ptr outer1 = std::make_unique<Window>(700, 400, bgcolor);
	std::shared_ptr outer2 = std::make_unique<Window>(700, 400, sf::Color(0x444444cc));
	std::shared_ptr windowText = TextDisplay::DefaultText("WindowText", 24);
	std::shared_ptr inner1 = std::make_unique<Window>(200, 200, sf::Color::Black);
	std::shared_ptr inner2 = std::make_unique<Window>(600, 200, sf::Color::Black);

	outer1->setHeader(30, "Outer1");
	outer1->getContentFrame()->addChildAutoPos(windowText);


	outer2->setHeader(30, "Outer2");

	inner1->getContentFrame()->addChildAutoPos(
		TextDisplay::Multiline("cat... cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat", 100, 24)
	);

	std::shared_ptr menu1 = std::make_unique<MenuOption>(" Option1 ", 15);
	auto button1 = std::make_shared<MenuOption>("This works! :)", 30);
	button1->setNormalColor(sf::Color::Magenta);
	button1->setTextColor(sf::Color::Black);
	menu1->addChild( button1, 0, 30);

	std::shared_ptr menu2 = std::make_unique<MenuOption>(" Option2 ", 15);
	menu2->addChild( std::make_shared<MenuOption>("This works! :)", 30) );

	outer1->setMenuBar(30);
	outer1->addMenuOption(menu1);
	outer1->addMenuOption(menu2);

	outer1->getContentFrame()->addChildAutoPos(inner1);
	outer1->getContentFrame()->addChildAutoPos(inner2);
	inner1->setHeader(30, "Inner1");
	inner2->setHeader(30, "Inner2");

	outer2->setMenuBar(30);
	using pos_t = MenuOption::OptionList::ChildPos_t;
	auto q = MenuOption::createMenu(
		100, 30, 15, {
			"Menu1", pos_t::Down, {{
				"inner1^", inner1}, {
				"Menu12", {
					{"Menu121"},
					{"Menu122"},
					{"outer1^", outer1},
				}},
			}
		}
	);
	outer2->getMenuFrame()->addChildAutoPos( q );

	auto notes = generateNotes(0, 2);

	auto generator = DynamicToneSum(Sine13, notes, 5);
	auto pitch = PitchBender<DynamicToneSum>();
	auto pitchWindow = std::make_shared<Window>(pitch.getFrame());
	outer2->getContentFrame()->addChildAutoPos(pitchWindow);

	auto glider = Glider(Sine13, notes, 5);
	auto gliderWindow = std::make_shared<Window>(glider.getFrame());
	outer2->getContentFrame()->addChildAutoPos(gliderWindow);
	

///// Text display
	std::shared_ptr test = TextDisplay::Multiline("The quick brown fox jumps over the lazy dog", 50, 24);
	std::shared_ptr test2 = TextDisplay::DefaultText("teeesztqq", 24);
	std::shared_ptr test3 = std::make_unique<TextDisplay>("qweasd", 100, 100, 24);

	test2->centralize();
	test->setBgColor({ 0,0,0,0 });
	test->fitFrame();

///// Event handling
	auto events = std::make_shared<EmptyGuiElement>([=](const SynthEvent & eventArg) {
		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			//const auto& gliderWindow = outer2;
			switch (event.type) {
			case sf::Event::KeyPressed: {
				switch (event.key.code) {
				case sf::Keyboard::X: {
					// For testing
					gliderWindow->setSize(gliderWindow->getSize()+SynthVec2(0,1));
					std::cout << gliderWindow->getSize().x << " " << gliderWindow->getSize().y << "\n";
					break;
				}
				case sf::Keyboard::Z: {
					// For testing
					gliderWindow->setSize(gliderWindow->getSize() + SynthVec2(1, 0));
					std::cout << gliderWindow->getSize().x << " " << gliderWindow->getSize().y << "\n";
					break;
				}
				default:
					break;
				}
				break;
			}
			default: {
				break;
			}
					 break;
			}
		}
		});
	
	gui->addChild(test);
	gui->addChild(test2);
	gui->addChild(test3);
	gui->addChild(events);
	gui->addChild(outer1, 50, 270);
	gui->addChild(outer2, 800, 370);

	test->moveAroundPoint({ 625,290 });
	test2->moveAroundPoint({ 70,70 });
	test3->moveAroundPoint({ 700, 700 });
	
}

int testMain(int argc, char** argv)
{
	auto gui = std::make_shared<EmptyGuiElement>();
	auto keyboard = KeyboardOutput();

	MidiContext midiContext;

	testGui(gui);

	sf::RenderWindow window(sf::VideoMode(1600, 1000), "Basic synth");
	setupGui(gui, window);
	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);
	while (window.isOpen()) {
		static sf::Event event;
		static MidiEvent midiEvent;

		while (midiContext.pollEvent(midiEvent)) {
			gui->forwardEvent(midiEvent);
		}
		while (window.pollEvent(event)) {
			gui->forwardEvent(event);
		}

		window.clear(sf::Color::Black);
		window.draw(*gui);
		window.display();
	}

	return 0;
}