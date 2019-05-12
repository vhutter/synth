#include "test.h"

#include <memory>
#include "../gui/GuiElements.h"
#include "../core/Instrument.h"

void testGui()
{
	auto gui = std::make_shared<EmptyGuiElement>();

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
		30, 15, {
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

	static auto generator = DynamicToneSum(Sines1(), ADSREnvelope(), notes, 5);
	
	static auto pitch = PitchBender<DynamicToneSum>(generator);
	auto pitchWindow = std::make_shared<Window>(pitch.getFrame());
	outer2->getContentFrame()->addChildAutoPos(pitchWindow);

	auto glider = Glider(Sines1(), notes, 5);
	auto gliderWindow = std::make_shared<Window>(glider.getFrame());
	outer2->getContentFrame()->addChildAutoPos(gliderWindow);

	auto input = std::make_shared<InputField>(InputField::Double, 100, 30);
	outer2->getContentFrame()->addChildAutoPos(input);

	auto randomSlider = std::shared_ptr(Slider::DefaultSlider("RandomSlider", -1, 1));
	outer2->getContentFrame()->addChild(randomSlider, 300, 140);
	outer2->getContentFrame()->addChild(randomSlider->getConfigFrame(), 300, 140);


	auto inputRec = std::make_shared<InputRecord>(InputRecord::Any, 60, 30, 15);
	outer2->getContentFrame()->addChild(inputRec, 100, 100);

	

///// Text display
	std::shared_ptr test = TextDisplay::Multiline("The quick brown fox jumps over the lazy dog", 50, 24);
	std::shared_ptr test2 = TextDisplay::DefaultText("teeesztqq", 24);
	std::shared_ptr test3 = std::make_unique<TextDisplay>("qweasd", 100, 100, 24);

	sf::String asd("asd");
	std::string sasd(asd);

	test2->centralize();
	test->setBgColor({ 0,0,0,0 });
	test->fitFrame();

///// Event handling
	auto events = std::make_shared<EmptyGuiElement>(
	[=](const sf::Event & event) {
		//const auto& gliderWindow = outer2;
		switch (event.type) {
		case sf::Event::KeyPressed: {
			switch (event.key.code) {
			case sf::Keyboard::X: {
				//gliderWindow->setSize(gliderWindow->getSize()+SynthVec2(0,1));
				//std::cout << gliderWindow->getSize().x << " " << gliderWindow->getSize().y << "\n";
				break;
			}
			case sf::Keyboard::Z: {
				//gliderWindow->setSize(gliderWindow->getSize() + SynthVec2(1, 0));
				//std::cout << gliderWindow->getSize().x << " " << gliderWindow->getSize().y << "\n";
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
	}, [=](const MidiEvent& event) {
		if (event.getType() == MidiEvent::Type::WHEEL) {
			double newValue = double(event.getWheelValue()) / MidiEvent::wheelValueMax() * 2. - 1;
			//randomSlider->setValue(newValue);
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

	sf::RenderWindow window(sf::VideoMode(1600, 1000), "Basic synth");

	window.setKeyRepeatEnabled(false);
	window.setVerticalSyncEnabled(true);
	MidiContext midiContext;
	while (window.isOpen()) {
		static sf::Event event;
		static MidiEvent midiEvent;

		while (midiContext.pollEvent(midiEvent)) {
			gui->forwardEvent(midiEvent);
		}
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed: {
				window.close();
				break;
			}
			default:
				break;
			}
			gui->forwardEvent(event);
		}

		window.clear(sf::Color::Black);
		window.draw(*gui);
		window.display();
	}
}