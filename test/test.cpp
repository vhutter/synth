#include "test.h"
#include "../gui.h"
#include "../guiElements.h"

void testGui(GuiElement& gui)
{
	sf::Color bgcolor = sf::Color::Magenta;
	bgcolor.a = 100;
	std::shared_ptr outer1 = std::make_unique<Window>(700, 400, bgcolor);
	std::shared_ptr outer2 = std::make_unique<Window>(700, 400, bgcolor);
	std::shared_ptr windowText = TextDisplay::DefaultText("WindowText", 0, 0, 24);
	std::shared_ptr inner1 = std::make_unique<Window>(200, 200, sf::Color::Black);
	std::shared_ptr inner2 = std::make_unique<Window>(600, 200, sf::Color::Black);

	outer1->setHeader(30, "Outer1");
	outer1->setPosition(50, 270);
	outer1->getContentFrame()->addChildrenAutoPos({
		windowText,
		});


	outer2->setPosition(50, 370);
	outer2->setHeader(30, "Outer2");

	inner1->getContentFrame()->addChildrenAutoPos({
		TextDisplay::Multiline("cat... cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat", 0, 0, 100, 24)
		});

	std::shared_ptr menu1 = std::make_unique<MenuOption>(" Option1 ", 15);
	auto button1 = std::shared_ptr(Button::DefaultButton("This works! :)", 0, 30, []() {std::cout << "button1 pressed\n"; }));
	button1->setBgColor(sf::Color::Magenta);
	button1->setTextColor(sf::Color::Black);
	menu1->addChildren({ button1 });

	std::shared_ptr menu2 = std::make_unique<MenuOption>(" Option2 ", 15);
	menu2->addChildren({TextDisplay::DefaultText("This works! :)", 50, 80, 50)});

	outer1->setMenuBar(30);
	outer1->addMenuOption(menu1);
	outer1->addMenuOption(menu2);

	outer1->getContentFrame()->addChildrenAutoPos({ inner1, inner2 });
	inner1->setHeader(30, "Inner1");
	inner2->setHeader(30, "Inner2");

	auto events = std::make_shared<EmptyGuiElement>([=](const SynthEvent& eventArg) {
		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			switch (event.type) {
				case sf::Event::KeyPressed: {
					switch (event.key.code) {
					case sf::Keyboard::X: {
						// For testing
						outer1->setSize(SynthVec2(10, 10) + outer1->getSize());
						break;
					}
					case sf::Keyboard::Z: {
						// For testing
						static int v = 0;
						outer1->setVisibility((v++) % 2);
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

	std::shared_ptr test = TextDisplay::Multiline("The quick brown fox jumps over the lazy dog", 600, 200, 50, 24);
	std::shared_ptr test2 = TextDisplay::DefaultText("teeesztqq", 250, 200, 24);
	std::shared_ptr test3 = std::make_unique<TextDisplay>("qweasd", 700, 700, 100, 100, 24);
	test2->centralize();
	test2->moveAroundPoint({ 70,70 });
	test->moveAroundPoint({ 625,290 });
	test->setBgColor({ 0,0,0,0 });
	test->fitFrame();

	gui.addChildren({
		//std::make_shared<Window>(10, 10, 500, 500, sf::Color{ 0x8b, 0, 0, 100 }) ,
		test,
		test2,
		test3,
		events,
		outer1,
		outer2,
	});
}

int testMain(int argc, char** argv)
{
	EmptyGuiElement gui;
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
			gui.forwardEvent(midiEvent);
		}
		while (window.pollEvent(event)) {
			gui.forwardEvent(event);
		}

		window.clear(sf::Color::Black);
		window.draw(gui);
		window.display();
	}

	return 0;
}