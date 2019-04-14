#include "test.h"
#include "../gui.h"
#include "../guiElements.h"

void testGui(GuiElement& gui)
{
	sf::Color bgcolor = sf::Color::Magenta;
	bgcolor.a = 100;
	std::shared_ptr testWindow = std::make_unique<Window>(700, 400, bgcolor);
	std::shared_ptr windowText = TextDisplay::DefaultText("WindowText", 0, 0, 24);
	std::shared_ptr testWindow2 = std::make_unique<Window>(200, 200, sf::Color::Black);
	std::shared_ptr testWindow3 = std::make_unique<Window>(600, 200, sf::Color::Black);

	testWindow->setHeader(30, "OuterWindow");
	testWindow2->setHeader(30, "Inner1");
	testWindow3->setHeader(30, "Inner2");

	testWindow->setPosition(50, 270);
	//testWindow2->setPosition(200, 100);
	//testWindow3->setPosition(300, 130);

	windowText->setPosition(50, 50);
	testWindow2->addChildren({
		TextDisplay::Multiline("cat... cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat cat", 0, 0, 100, 24)
		});
	testWindow->setCursor(0, 100);
	testWindow->addChildrenAutoPos({
		windowText,
		testWindow2,
		testWindow3,
		});
	testWindow->setMenuBar(30);

	std::shared_ptr menu1 = std::make_unique<MenuOption>(" Option1 ", 15);
	menu1->addChildren({TextDisplay::DefaultText("This works! :)", 50, 50, 50)});

	std::shared_ptr menu2 = std::make_unique<MenuOption>(" Option2 ", 15);
	menu2->addChildren({TextDisplay::DefaultText("This works! :)", 50, 80, 50)});

	testWindow->addMenuOption(menu1);
	testWindow->addMenuOption(menu2);

	auto events = std::make_shared<EmptyGuiElement>([=](const SynthEvent& eventArg) {
		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			switch (event.type) {
				case sf::Event::KeyPressed: {
					switch (event.key.code) {
					case sf::Keyboard::X: {
						// For testing
						testWindow->setSize(SynthVec2(10,10) + testWindow->getSize());
						break;
					}
					case sf::Keyboard::Z: {
						// For testing
						static int v = 0;
						testWindow->setVisibility((v++) % 2);
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

	//auto test = std::make_shared<TextDisplay>("The quick brown fox jumps over the lazy dog", 600, 200, 300, 300);
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
		testWindow,
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